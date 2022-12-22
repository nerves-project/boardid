// SPDX-FileCopyrightText: 2020 Nerves Project Developers
//
// SPDX-License-Identifier: Apache-2.0

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
