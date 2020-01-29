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
    int fd = atecc508a_open(options->filename);
    if (fd < 0)
        return false;

    if (atecc508a_wakeup(fd) < 0) {
        atecc508a_close(fd);
        return false;
    }

    uint8_t serial_number[9];
    if (atecc508a_read_serial(fd, serial_number) < 0) {
        atecc508a_close(fd);
        return false;
    }

    atecc508a_sleep(fd);

    atecc508a_close(fd);

    char serial_number_hex[sizeof(serial_number) * 2 + 1];
    bin_to_hex(serial_number, sizeof(serial_number), serial_number_hex);

    // The ATECC508A has a 18 character serial number (9 bytes)
    memcpy(buffer, serial_number_hex, sizeof(serial_number_hex));
    buffer[sizeof(serial_number_hex)] = 0;
    return true;
}
