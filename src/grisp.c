// SPDX-FileCopyrightText: 2022 Nerves Project Developers
//
// SPDX-License-Identifier: Apache-2.0

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
