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

// See https://github.com/fhunleth/atecc508a for Nerves Key OTP format

static uint8_t nerves_key_magic[4] = {0x4e, 0x72, 0x76, 0x73};

bool nerves_key_id(const struct boardid_options *options, char *buffer)
{
    int fd = atecc508a_open(options->filename);
    if (fd < 0)
        return 0;

    if (atecc508a_wakeup(fd) < 0) {
        atecc508a_close(fd);
        return false;
    }

    // The Nerves Key's serial number is in the OTP memory
    uint8_t otp[32];
    if (atecc508a_read_zone_nowake(fd, ATECC508A_ZONE_OTP, 0, 0, 0, otp, 32) < 0 &&
        atecc508a_read_zone_nowake(fd, ATECC508A_ZONE_OTP, 0, 0, 0, otp, 32) < 0) {
        atecc508a_sleep(fd);
        atecc508a_close(fd);
        return false;
    }

    atecc508a_sleep(fd);
    atecc508a_close(fd);

    if (memcmp(otp, nerves_key_magic, sizeof(nerves_key_magic)) != 0)
        return false;

    const char *serial_number = (const char *) &otp[16];
    size_t serial_number_len = strnlen(serial_number, 16);

    memcpy(buffer, serial_number, serial_number_len);
    buffer[serial_number_len] = 0;
    return true;
}
