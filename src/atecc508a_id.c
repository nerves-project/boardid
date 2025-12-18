// SPDX-FileCopyrightText: 2018 Frank Hunleth
//
// SPDX-License-Identifier: Apache-2.0

#include "common.h"
#include "atecc508a.h"

#include <string.h>

bool atecc508a_id(const struct boardid_options *options, char *buffer)
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

    uint8_t serial_number[9];
    if (atecc508a_read_serial(&atecc, serial_number) < 0) {
        atecc508a_close(&atecc);
        return false;
    }

    atecc508a_sleep(&atecc);

    atecc508a_close(&atecc);

    char serial_number_hex[FORMAT_BINARY_OUTPUT_LENGTH(sizeof(serial_number))];
    format_binary_data(serial_number, sizeof(serial_number), options->output_format, serial_number_hex);

    // The ATECC508A has a 18 character serial number (9 bytes)
    memcpy(buffer, serial_number_hex, sizeof(serial_number_hex));
    buffer[sizeof(serial_number_hex)] = 0;
    return true;
}
