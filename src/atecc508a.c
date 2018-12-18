/*
   Copyright 2018 Frank Hunleth

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "atecc508a.h"

#define ATECC508A_ADDR 0x60
#define ATECC508A_WAKE_DELAY_US 1500

#include <err.h>
#define ERROR warnx
#define INFO(...)

// The ATECC508A/608A have different times for how long to wait for commands to complete.
// Unless I'm totally misreading the datasheet (Table 9-4), it really seems like some are too short.
// See https://github.com/MicrochipTech/cryptoauthlib/blob/master/lib/atca_execution.c#L98 for
// another opinion on execution times.
//
// I'm taking the "typical" time from the datasheet and the "max" time from the longest
// I see on the 508A/608A in atca_execution.c or the datasheet.

struct atecc508a_opcode_info {
    uint8_t opcode;  // Opcode
    uint16_t length; // Response length
    int typical_us;  // Typical processing time
    int max_us;      // Max processing time
};

//static const struct atecc508a_opcode_info op_checkmac =   {0x28,  1,  5000,  40000};
//static const struct atecc508a_opcode_info op_derive_key = {0x1c,  1,  2000,  50000};
//static const struct atecc508a_opcode_info op_ecdh =       {0x43,  1, 38000, 531000};
static const struct atecc508a_opcode_info op_genkey =     {0x40, 64, 11000, 653000};
static const struct atecc508a_opcode_info op_nonce =      {0x16,  1,   100,  29000};
static const struct atecc508a_opcode_info op_read4 =      {0x02,  4,   100,   5000};
static const struct atecc508a_opcode_info op_read32 =     {0x02, 32,   100,   5000};
static const struct atecc508a_opcode_info op_sign =       {0x41, 64, 42000, 665000};

static int microsleep(int microseconds)
{
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = microseconds * 1000;
    int rc;

    while ((rc = nanosleep(&ts, &ts)) < 0 && errno == EINTR);

    return rc;
}

static int i2c_read(int fd, uint8_t addr, uint8_t *to_read, size_t to_read_len)
{
    struct i2c_rdwr_ioctl_data data;
    struct i2c_msg msg;

    msg.addr = addr;
    msg.flags = I2C_M_RD;
    msg.len = (uint16_t) to_read_len;
    msg.buf = to_read;
    data.msgs = &msg;
    data.nmsgs = 1;

    return ioctl(fd, I2C_RDWR, &data);
}

static int i2c_write(int fd, uint8_t addr, const uint8_t *to_write, uint16_t to_write_len)
{
    struct i2c_rdwr_ioctl_data data;
    struct i2c_msg msg;

    msg.addr = addr;
    msg.flags = 0;
    msg.len = (uint16_t) to_write_len;
    msg.buf = (uint8_t *) to_write;
    data.msgs = &msg;
    data.nmsgs = 1;

    return ioctl(fd, I2C_RDWR, &data);
}

static int i2c_poll_read(int fd, uint8_t addr, uint8_t *to_read, size_t to_read_len, int min_us, int max_us)
{
    int amount_slept = min_us;
    int poll_interval = 1000;
    int rc;

    microsleep(min_us);

    do {
        rc = i2c_read(fd, addr, to_read, to_read_len);
        if (rc >= 0)
            break;

        microsleep(poll_interval);
        amount_slept += poll_interval;
    } while (amount_slept <= max_us);
    return rc;
}

// See Atmel CryptoAuthentication Data Zone CRC Calculation application note
static void atecc508a_crc(uint8_t *data)
{
    const uint16_t polynom = 0x8005;
    uint16_t crc_register = 0;

    size_t length = data[0] - 2;

    for (size_t counter = 0; counter < length; counter++)
    {
        for (uint8_t shift_register = 0x01; shift_register > 0x00; shift_register <<= 1)
        {
            uint8_t data_bit = (data[counter] & shift_register) ? 1 : 0;
            uint8_t crc_bit = crc_register >> 15;
            crc_register <<= 1;
            if (data_bit != crc_bit)
                crc_register ^= polynom;
        }
    }

    uint8_t *crc_le = &data[length];
    crc_le[0] = (uint8_t)(crc_register & 0x00FF);
    crc_le[1] = (uint8_t)(crc_register >> 8);
}

static int atecc508a_request(int fd, const struct atecc508a_opcode_info *op, uint8_t *msg, uint8_t *response)
{
    atecc508a_crc(&msg[1]);

    // Calculate and append the CRC and send
    if (i2c_write(fd, ATECC508A_ADDR, msg, msg[1] + 1) < 0) {
        ERROR("Error from i2c_write for opcode 0x%02x", msg[2]);
        return -1;
    }

    if (i2c_poll_read(fd, ATECC508A_ADDR, response, op->length + 3, op->typical_us, op->max_us) < 0) {
        ERROR("Error for i2c_read for opcode 0x%02x. Waited %d us", msg[2], op->max_us);
        return -1;
    }

    // Check length
    if (response[0] != op->length + 3) {
        ERROR("Response error for opcode 0x%02x: 0x%02x", msg[2], response[0]);
        return -1;
    }

    // Check the CRC
    uint8_t got_crc[2];
    got_crc[0] = response[op->length + 1];
    got_crc[1] = response[op->length + 2];
    atecc508a_crc(response);
    if (got_crc[0] != response[op->length + 1] || got_crc[1] != response[op->length + 2]) {
        ERROR("CRC error for opcode 0x%02x", msg[2]);
        return -1;
    }

    return 0;
}

int atecc508a_open(const char *filename)
{
    return open(filename, O_RDWR);
}

void atecc508a_close(int fd)
{
    close(fd);
}

int atecc508a_wakeup(int fd)
{
    // See ATECC508A 6.1 for the wakeup sequence.
    //
    // Write to address 0 to pull SDA down for the wakeup interval (60 uS).
    // Since only 8-bits get through, the I2C speed needs to be < 133 KHz for
    // this to work.
    uint8_t zero = 0;
    i2c_write(fd, 0, &zero, 1);

    // Wait for the device to wake up for real
    microsleep(ATECC508A_WAKE_DELAY_US);

    // Check that it's awake by reading its signature
    uint8_t buffer[4];
    if (i2c_read(fd, ATECC508A_ADDR, buffer, sizeof(buffer)) < 0) {
        ERROR("Can't wakeup ATECC508A");
        return -1;
    }

    if (buffer[0] != 0x04 ||
            buffer[1] != 0x11 ||
            buffer[2] != 0x33 ||
            buffer[3] != 0x43) {
        ERROR("Bad ATECC508A signature: %02x%02x%02x%02x", buffer[0], buffer[1], buffer[2], buffer[3]);
        return -1;
    }

    return 0;
}

int atecc508a_sleep(int fd)
{
    // See ATECC508A 6.2 for the sleep sequence.
    uint8_t sleep = 0x01;
    if (i2c_write(fd, ATECC508A_ADDR, &sleep, 1) < 0)
        return -1;

    return 0;
}

static int atecc508a_get_addr(uint8_t zone, uint16_t slot, uint8_t block, uint8_t offset, uint16_t *addr)
{
    switch (zone) {
    case ATECC508A_ZONE_CONFIG:
    case ATECC508A_ZONE_OTP:
        *addr = (uint16_t) (block << 3) + (offset & 7);
        return 0;

    case ATECC508A_ZONE_DATA:
        *addr = (uint16_t) ((block << 8) + (slot << 3) + (offset & 7));
        return 0;

    default:
        return -1;
    }
}

/**
 * Read data out of a zone
 *
 * @param fd the fd opened by atecc508a_open
 * @param zone ATECC508A_ZONE_CONFIG, ATECC508A_ZONE_OTP, or ATECC508A_ZONE_DATA
 * @param slot the slot if this is a data zone
 * @param block which block
 * @param offset the offset in the block
 * @param data where to store the data
 * @param len how much to read (4 or 32 bytes)
 * @return 0 on success
 */
int atecc508a_read_zone_nowake(int fd, uint8_t zone, uint16_t slot, uint8_t block, uint8_t offset, uint8_t *data, uint8_t len)
{
    uint16_t addr;

    if (atecc508a_get_addr(zone, slot, block, offset, &addr) < 0)
        return -1;

    uint8_t zone_flag;
    const struct atecc508a_opcode_info *op;
    switch (len) {
    case 32:
        zone_flag = 0x80;
        op = &op_read32;
        break;

    case 4:
        zone_flag = 0x00;
        op = &op_read4;
        break;

    default:
        ERROR("Bad read length %d", len);
        return -1;
    }

    uint8_t msg[8];
    msg[0] = 3;    // "word address"
    msg[1] = 7;    // 7 byte message
    msg[2] = 0x02; // Read opcode
    msg[3] = zone_flag | zone;
    msg[4] = addr & 0xff;
    msg[5] = addr >> 8;

    uint8_t response[32 + 3];
    if (atecc508a_request(fd, op, msg, response) < 0)
        return -1;

    // Copy the data (bytes after the count field)
    memcpy(data, &response[1], len);

    return 0;
}

/**
 * Read the ATECC508A's serial number
 *
 * @param fd the fd opened by atecc508a_open
 * @param serial_number a 9-byte buffer for the serial number
 * @return 0 on success
 */
int atecc508a_read_serial(int fd, uint8_t *serial_number)
{
    if (atecc508a_wakeup(fd) < 0)
        return -1;

    // Read the config -> try 2 times just in case there's a hiccup on the I2C bus
    uint8_t buffer[32];
    if (atecc508a_read_zone_nowake(fd, ATECC508A_ZONE_CONFIG, 0, 0, 0, buffer, 32) < 0 &&
            atecc508a_read_zone_nowake(fd, ATECC508A_ZONE_CONFIG, 0, 0, 0, buffer, 32) < 0)
        return -1;

    // Copy out the serial number (see datasheet for offsets)
    memcpy(&serial_number[0], &buffer[0], 4);
    memcpy(&serial_number[4], &buffer[8], 5);

    atecc508a_sleep(fd);
    return 0;
}

/**
 * Derive a public key from the private key that's stored in the specified slot.
 *
 * @param fd the fd opened by atecc508a_open
 * @param slot which slot
 * @param key a 64-byte buffer for the key
 * @return 0 on success
 */
int atecc508a_derive_public_key(int fd, uint8_t slot, uint8_t *key)
{
    // Send a GenKey command to derive the public key from a previously stored private key
    uint8_t msg[11];

    if (atecc508a_wakeup(fd) < 0)
        return -1;

    msg[0] = 3;    // "word address"
    msg[1] = 10;   // 10 byte message
    msg[2] = op_genkey.opcode;
    msg[3] = 0;    // Mode
    msg[4] = slot;
    msg[5] = 0;
    msg[6] = 0;
    msg[7] = 0;
    msg[8] = 0;

    uint8_t response[64 + 3];
    if (atecc508a_request(fd, &op_genkey, msg, response) < 0)
        return -1;

    // Copy the data (bytes after the count field)
    memcpy(key, &response[1], 64);

    atecc508a_sleep(fd);

    return 0;
}

/**
 * Derive a public key from the private key that's stored in the specified slot.
 *
 * @param fd the fd openned by atecc508a_open
 * @param slot which slot
 * @param data a 32-byte input buffer to sign
 * @param signature a 64-byte buffer for the signature
 * @return 0 on success
 */
int atecc508a_sign(int fd, uint8_t slot, const uint8_t *data, uint8_t *signature)
{
    // Send a Nonce command to load the data into TempKey
    uint8_t msg[40];

    if (atecc508a_wakeup(fd) < 0)
        return -1;

    msg[0] = 3;    // "word address"
    msg[1] = 39;   // Length
    msg[2] = op_nonce.opcode;
    msg[3] = 0x3;  // Mode - Write NumIn to TempKey
    msg[4] = 0;    // Zero LSB
    msg[5] = 0;    // Zero MSB
    memcpy(&msg[6], data, 32); // NumIn

    uint8_t response[64 + 3];
    if (atecc508a_request(fd, &op_nonce, msg, response) < 0)
        return -1;

    if (response[1] != 0) {
        INFO("Unexpected Nonce response %02x %02x %02x %02x", response[0], response[1], response[2], response[3]);
        return -1;
    }

    // Sign the value in TempKey
    msg[0] = 3;     // "word address"
    msg[1] = 7;     // Length
    msg[2] = op_sign.opcode;
    msg[3] = 0x80;  // Mode - the data to be signed is in TempKey
    msg[4] = slot;  // KeyID LSB
    msg[5] = 0;     // KeyID MSB

    if (atecc508a_request(fd, &op_sign, msg, response) < 0)
        return -1;

    // Copy the data (bytes after the count field)
    memcpy(signature, &response[1], 64);

    atecc508a_sleep(fd);

    return 0;
}
