/*
   Copyright 2022 Nerves Project Developers

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

// Read the serial number from the GRiSP2's EEPROM
// See https://github.com/grisp/grisp/blob/master/src/grisp_hw.erl
bool grisp_id(const struct boardid_options *options, char *buffer)
{
    FILE *fp = fopen_helper("/sys/bus/i2c/devices/0-0057/eeprom", "r");
    if (!fp)
        return 0;

    unsigned char data[28];
    if (fread(data, 1, sizeof(data), fp) != sizeof(data)) {
        fclose(fp);
        return false;
    }

    fclose(fp);

    // The GRiSP serial is a little endian (native endian on GRiSP devices)
    // 32-bit integer
    const uint32_t *serial = (const uint32_t *) &data[4];
    sprintf(buffer, "%d", *serial);
    return true;
}
