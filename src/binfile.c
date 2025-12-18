// SPDX-FileCopyrightText: 2016 Frank Hunleth
//
// SPDX-License-Identifier: Apache-2.0

#include <string.h>
#include <stdlib.h>

#include "common.h"

bool binfile_id(const struct boardid_options *options, char *buffer)
{
    int idlen = options->size;
    if (idlen <= 0 || idlen > MAX_SERIALNUMBER_LEN)
        return false;

    FILE *fp = fopen_helper(options->filename, "r");
    if (!fp)
        return false;

    if (fseek(fp, options->offset, SEEK_SET) < 0) {
        fclose(fp);
        return false;
    }

    uint8_t data[idlen];
    if (fread(data, 1, idlen, fp) != idlen) {
        fclose(fp);
        return false;
    }

    fclose(fp);

    format_binary_data(data, idlen, options->output_format, buffer);

    return true;
}

bool linkit_id(const struct boardid_options *options, char *buffer)
{
    // Use the LinkIt Smart's WLAN MAC address for the unique ID.
    struct boardid_options linkit_options = *options;
    linkit_options.filename = "/dev/mtdblock2";
    linkit_options.size = 6;
    linkit_options.offset = 0x4;

    return binfile_id(&linkit_options, buffer);
}

// Read the serial number from the GRiSP2's EEPROM
// See https://github.com/grisp/grisp/blob/master/src/grisp_hw.erl
bool grisp_id(const struct boardid_options *options, char *buffer)
{
    struct boardid_options grisp2_options = *options;
    grisp2_options.filename = "/sys/bus/i2c/devices/0-0057/eeprom";
    grisp2_options.size = 4;
    grisp2_options.offset = 0x4;
    grisp2_options.output_format = OUTPUT_FORMAT_LE_DECIMAL;

    return binfile_id(&grisp2_options, buffer);
}
