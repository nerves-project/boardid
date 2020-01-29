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

#include <string.h>
#include <stdlib.h>

#include "common.h"

// Read the serial number from the Beaglebone's EEPROM
// See the SRM
bool beagleboneblack_id(const struct boardid_options *options, char *buffer)
{
    // Try both the Linux 3.8 and 4.1 EEPROM locations
    FILE *fp = fopen_helper("/sys/bus/i2c/devices/0-0050/eeprom", "r");
    if (!fp)
        fp = fopen_helper("/sys/bus/i2c/devices/0-0050/at24-0/nvmem", "r");

    if (!fp)
        return 0;

    unsigned char data[28];
    if (fread(data, 1, sizeof(data), fp) != sizeof(data)) {
        fclose(fp);
        return false;
    }

    fclose(fp);

    // Verify that the EEPROM was programmed
    if (data[0] != 0xaa ||
            data[1] != 0x55 ||
            data[2] != 0x33 ||
            data[3] != 0xee)
        return false;


    // The BBB has a 12 character serial number
    const int digits = 12;
    memcpy(buffer, &data[16], digits);
    buffer[digits] = 0;
    return true;
}
