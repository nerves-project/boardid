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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"

// Read the serial number from /proc/cpuinfo.
// The Raspberry Pi and Lego EV3 report their serial number using this mechanism.

bool cpuinfo_id(const struct boardid_options *options, char *buffer)
{
    FILE *fp = fopen_helper("/proc/cpuinfo", "r");
    if (!fp)
        return false;

    char line[256];
    const char *serial = NULL;
    while (fgets(line, sizeof(line), fp)) {
        static const char to_match[] = "Serial\t\t: ";
        if (memcmp(line, to_match, sizeof(to_match) - 1) == 0) {
            serial = line + sizeof(to_match) - 1;
            break;
        }
    }
    fclose(fp);

    // Make sure that the serial number was found.
    if (!serial)
        return false;

    // Copy the serial number to the output buffer
    int digits = strlen(serial) - 1; // -1 is for the '\n' at the end of the line
    if (digits > MAX_SERIALNUMBER_LEN)
        digits = MAX_SERIALNUMBER_LEN;

    memcpy(buffer, serial, digits);
    buffer[digits] = '\0';

    return true;
}
