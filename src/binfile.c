// SPDX-FileCopyrightText: 2020 Nerves Project Developers
//
// SPDX-License-Identifier: Apache-2.0

#include <string.h>
#include <stdlib.h>

#include "common.h"

#define MAX_BINFILE_ID_LEN (MAX_SERIALNUMBER_LEN / 2)

bool binfile_id(const struct boardid_options *options, char *buffer)
{
    FILE *fp = fopen_helper(options->filename, "r");
    if (!fp)
        return false;

    if (fseek(fp, options->offset, SEEK_SET) < 0) {
        fclose(fp);
        return false;
    }

    int idlen = options->size;
    if (idlen > MAX_BINFILE_ID_LEN)
        idlen = MAX_BINFILE_ID_LEN;
    uint8_t data[MAX_BINFILE_ID_LEN];

    if (fread(data, 1, idlen, fp) != idlen) {
        fclose(fp);
        return false;
    }

    fclose(fp);

    bin_to_hex(data, idlen, options->capital_hex, buffer);
    buffer[idlen * 2] = '\0';

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
