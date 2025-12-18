// SPDX-FileCopyrightText: 2020 Frank Hunleth
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"

// Read the serial number from the device tree. Many devices
// place it here including Raspberry Pi's and nVIDIA Jetson's.
bool device_tree_id(const struct boardid_options *options, char *buffer)
{
    FILE *fp = fopen_helper("/sys/firmware/devicetree/base/serial-number", "r");
    if (!fp) {
        // Try the legacy path
        fp = fopen_helper("/proc/device-tree/serial-number", "r");
        if (!fp)
            return false;
    }

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
