// SPDX-FileCopyrightText: 2018 Frank Hunleth
//
// SPDX-License-Identifier: Apache-2.0

#include "common.h"
#include "atecc508a.h"

#include <string.h>

// See https://github.com/fhunleth/atecc508a for Nerves Key OTP format

static uint8_t nerves_key_magic[4] = {0x4e, 0x72, 0x76, 0x73};

static int retry_read_otp(const struct atecc508a_session *session, int block, uint8_t *dest)
{
    if (atecc508a_read_zone_nowake(session, ATECC508A_ZONE_OTP, 0, block, 0, dest, 32) < 0
        && atecc508a_read_zone_nowake(session, ATECC508A_ZONE_OTP, 0, block, 0, dest, 32) < 0)
        return -1;
    else
        return 0;
}

bool nerves_key_id(const struct boardid_options *options, char *buffer)
{
    uint8_t i2c_address = ATECC508A_DEFAULT_ADDR;
    if (options->i2c_address > 0 && options->i2c_address < 128)
        i2c_address = (uint8_t) options->i2c_address;

    struct atecc508a_session atecc;
    if (atecc508a_open(options->filename, i2c_address, &atecc) < 0)
        return false;

    if (atecc508a_wakeup(&atecc) < 0) {
        atecc508a_close(&atecc);
        return false;
    }

    // The Nerves Key's serial number is in the OTP memory
    uint8_t otp[64];
    memset(otp, 0, sizeof(otp));

    bool rc = false;
    if (retry_read_otp(&atecc, 0, otp) == 0) {
        int flags = (otp[4] << 8) + otp[5];
        size_t max_len = (flags & 1) ? 48 : 16;

        if (memcmp(otp, nerves_key_magic, sizeof(nerves_key_magic)) == 0
            && (max_len == 16 || retry_read_otp(&atecc, 1, &otp[32]) == 0)) {

            const char *serial_number = (const char *) &otp[16];
            size_t serial_number_len = strnlen(serial_number, max_len);

            memcpy(buffer, serial_number, serial_number_len);
            buffer[serial_number_len] = 0;
            rc = true;
        }
    }
    atecc508a_sleep(&atecc);
    atecc508a_close(&atecc);
    return rc;
}
