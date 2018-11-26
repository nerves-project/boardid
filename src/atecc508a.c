/*
   Copyright 2016 Frank Hunleth

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

#include "common.h"

#define ATECC508A_ADDR 0x60
#define ATECC508A_WAKE_DELAY_US 1500

#define ATECC508A_ZONE_CONFIG 0
#define ATECC508A_ZONE_OTP    1
#define ATECC508A_ZONE_DATA   2

static int microsleep(int microseconds)
{
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = microseconds * 1000;
    return nanosleep(&ts, NULL);
}

static int i2c_transfer(int fd,
                        unsigned int addr,
                        const uint8_t *to_write, size_t to_write_len,
                        uint8_t *to_read, size_t to_read_len)
{
    struct i2c_rdwr_ioctl_data data;
    struct i2c_msg msgs[2];

    msgs[0].addr = addr;
    msgs[0].flags = 0;
    msgs[0].len = to_write_len;
    msgs[0].buf = (uint8_t *) to_write;

    msgs[1].addr = addr;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len = to_read_len;
    msgs[1].buf = to_read;

    if (to_write_len != 0)
        data.msgs = &msgs[0];
    else
        data.msgs = &msgs[1];

    data.nmsgs = (to_write_len != 0 && to_read_len != 0) ? 2 : 1;

    return ioctl(fd, I2C_RDWR, &data);
}

static int atecc508a_wakeup(int fd)
{
    // See ATECC508A 6.1 for the wakeup sequence.
    //
    // Write to address 0 to pull SDA down for the wakeup interval (60 uS).
    // Since only 8-bits get through, the I2C speed needs to be < 133 KHz for
    // this to work.
    uint8_t zero = 0;
    i2c_transfer(fd, 0, &zero, 1, NULL, 0);

    // Wait for the device to wake up for real
    microsleep(ATECC508A_WAKE_DELAY_US);

    // Check that it's awake by reading its signature
    uint8_t buffer[4];
    if (i2c_transfer(fd, ATECC508A_ADDR, NULL, 0, buffer, sizeof(buffer)) < 0)
        return -1;

    if (buffer[0] != 0x04 ||
        buffer[1] != 0x11 ||
        buffer[2] != 0x33 ||
        buffer[3] != 0x43)
        return -1;

    return 0;
}

static int atecc508a_sleep(int fd)
{
    // See ATECC508A 6.2 for the sleep sequence.
    uint8_t sleep = 0x01;
    if (i2c_transfer(fd, ATECC508A_ADDR, &sleep, 1, NULL, 0) < 0)
        return -1;

    return 0;
}

static int atecc508a_get_addr(uint8_t zone, uint16_t slot, uint8_t block, uint8_t offset, uint16_t *addr)
{
    switch (zone) {
    case ATECC508A_ZONE_CONFIG:
    case ATECC508A_ZONE_OTP:
        *addr = (block << 3) + (offset & 7);
        return 0;

    case ATECC508A_ZONE_DATA:
        *addr = (block << 8) + (slot << 3) + (offset & 7);
        return 0;

    default:
        return -1;
    }
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

static int atecc508a_read_zone(int fd, uint8_t zone, uint16_t slot, uint8_t block, uint8_t offset, uint8_t *data, uint8_t len)
{
    uint16_t addr;

    if (atecc508a_get_addr(zone, slot, block, offset, &addr) < 0)
        return -1;

    uint8_t msg[8];

    msg[0] = 3;    // "word address"
    msg[1] = 7;    // 7 byte message
    msg[2] = 0x02; // Read opcode
    msg[3] = (len == 32 ? (zone | 0x80) : zone);
    msg[4] = addr & 0xff;
    msg[5] = addr >> 8;

    atecc508a_crc(&msg[1]);

    if (i2c_transfer(fd, ATECC508A_ADDR, msg, sizeof(msg), NULL, 0) < 0)
        return -1;

    // Wait for read to be available
    microsleep(5000);

    uint8_t response[32 + 3];
    if (i2c_transfer(fd, ATECC508A_ADDR, NULL, 0, response, len + 3) < 0)
        return -1;

    // Check length
    if (response[0] != len + 3)
        return -1;

    // Check the CRC
    uint8_t got_crc[2];
    got_crc[0] = response[len + 1];
    got_crc[1] = response[len + 2];
    atecc508a_crc(response);
    if (got_crc[0] != response[len + 1] || got_crc[1] != response[len + 2])
        return -1;

    // Copy the data (bytes after the count field)
    memcpy(data, &response[1], len);

    return 0;
}

static int atecc508a_read_serial(int fd, uint8_t *serial_number)
{
    // Read the config -> try 2 times just in case there's a hiccup on the I2C bus
    uint8_t buffer[32];
    if (atecc508a_read_zone(fd, ATECC508A_ZONE_CONFIG, 0, 0, 0, buffer, 32) < 0 &&
        atecc508a_read_zone(fd, ATECC508A_ZONE_CONFIG, 0, 0, 0, buffer, 32) < 0)
        return -1;

    // Copy out the serial number (see datasheet for offsets)
    memcpy(&serial_number[0], &buffer[0], 4);
    memcpy(&serial_number[4], &buffer[8], 5);
    return 0;
}

// Read the serial number from the Beaglebone's EEPROM
// See the SRM
int atecc508a_id(const struct id_options *options, char *buffer, int len)
{
    int fd = open(options->filename, O_RDWR);
    if (fd < 0)
        return 0;

    if (atecc508a_wakeup(fd) < 0) {
        close(fd);
        return 0;
    }

    uint8_t serial_number[9];
    if (atecc508a_read_serial(fd, serial_number) < 0) {
        close(fd);
        return 0;
    }

    atecc508a_sleep(fd);

    close(fd);

    char serial_number_hex[sizeof(serial_number) * 2 + 1];
    bin_to_hex(serial_number, sizeof(serial_number), serial_number_hex);

    // The ATECC508A has a 18 character serial number (9 bytes)
    if (len > sizeof(serial_number_hex))
        len = sizeof(serial_number_hex);
    if (len < 1)
        len = 1;

    int digits = len - 1;
    memcpy(buffer, serial_number_hex, digits);
    buffer[digits] = 0;
    return 1;
}
