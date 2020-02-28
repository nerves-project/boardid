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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"

// Read the serial number from /proc/device-tree/serial-number.
// The nVIDIA Jetson Nano and probably quite a few other devices
// report their IDs through here.

bool device_tree_id(const struct boardid_options *options, char *buffer)
{
    FILE *fp = fopen_helper("/proc/device-tree/serial-number", "r");
    if (!fp)
        return false;

    size_t len = fread(buffer, 1, MAX_SERIALNUMBER_LEN, fp);
    fclose(fp);

    // Strip any newlines off the end.
    for (size_t i = 0; i < len; i++) {
        if (buffer[i] == '\n') {
            len = i;
            break;
        }
    }
    buffer[len] = '\0';

    // Make sure that the serial number was found.
    return len > 0;
}
