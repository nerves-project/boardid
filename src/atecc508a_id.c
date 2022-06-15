/*
   Copyright 2020 Nerves Project Developers

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

    char serial_number_hex[sizeof(serial_number) * 2 + 1];
    bin_to_hex(serial_number, sizeof(serial_number), options->capital_hex, serial_number_hex);

    // The ATECC508A has a 18 character serial number (9 bytes)
    memcpy(buffer, serial_number_hex, sizeof(serial_number_hex));
    buffer[sizeof(serial_number_hex)] = 0;
    return true;
}
