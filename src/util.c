// SPDX-FileCopyrightText: 2016 Frank Hunleth
//
// SPDX-License-Identifier: Apache-2.0

#include <string.h>
#include <inttypes.h>

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

static void bin_to_hex(const uint8_t *input, size_t len, const char *format, char *output)
{
    for (size_t i = 0; i < len; i++)
        sprintf(&output[i * 2], format, input[i]);
}

static uint64_t from_be(const uint8_t *input, size_t len)
{
    if (len > sizeof(uint64_t)) len = sizeof(uint64_t);

    uint64_t value = 0;
    for (size_t i = 0; i < len; i++)
        value = (value << 8) | input[i];

    return value;
}

static uint64_t from_le(const uint8_t *input, size_t len)
{
    if (len > sizeof(uint64_t)) len = sizeof(uint64_t);

    uint64_t value = 0;
    for (size_t i = 0; i < len; i++)
        value |= (uint64_t)input[i] << (8 * i);

    return value;
}

void format_binary_data(const uint8_t *input, size_t len, enum output_format format, char *output)
{
    switch (format) {
    case OUTPUT_FORMAT_LOWERCASE_HEX:
        bin_to_hex(input, len, "%02x", output);
        break;
    case OUTPUT_FORMAT_UPPERCASE_HEX:
        bin_to_hex(input, len, "%02X", output);
        break;
    case OUTPUT_FORMAT_BE_DECIMAL:
        sprintf(output, "%" PRIu64, from_be(input, len));
        break;
    case OUTPUT_FORMAT_LE_DECIMAL:
        sprintf(output, "%" PRIu64, from_le(input, len));
        break;
    case OUTPUT_FORMAT_TEXT:
        memcpy(output, input, len);
        output[len] = '\0';
        break;
    }
}