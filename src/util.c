// SPDX-FileCopyrightText: 2020 Nerves Project Developers
//
// SPDX-License-Identifier: Apache-2.0

#include <string.h>

#include "common.h"

FILE *fopen_helper(const char *filename, const char *mode)
{
    if (filename == NULL || mode == NULL)
        return NULL;

    if (root_prefix == NULL) {
        // Normal case
        return fopen(filename, mode);
    } else {
        // Unit test case
        int prefix_len = strlen(root_prefix);
        int len = strlen(filename) + prefix_len + 1;
        char path[len];
        memcpy(path, root_prefix, prefix_len);
        strcpy(&path[prefix_len], filename);
        return fopen(path, mode);
    }
}

void bin_to_hex(const uint8_t *input, size_t len, int capital_hex, char *output)
{
    const char *format = capital_hex ? "%02X" : "%02x";
    for (size_t i = 0; i < len; i++)
        sprintf(&output[i * 2], format, input[i]);
}

