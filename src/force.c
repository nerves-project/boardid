// SPDX-FileCopyrightText: 2020 Nerves Project Developers
//
// SPDX-License-Identifier: Apache-2.0

#include "common.h"

#include <err.h>
#include <string.h>

bool force_id(const struct boardid_options *options, char *buffer)
{
    if (!options->filename) {
        warnx("specify -f to force an ID");
        return false;
    }

    strncpy(buffer, options->filename, MAX_SERIALNUMBER_LEN);
    buffer[MAX_SERIALNUMBER_LEN] = '\0';

    return buffer[0] != '\0';
}
