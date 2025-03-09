// SPDX-FileCopyrightText: 2020 Frank Hunleth
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"

// Read the serial number from /sys/firmware/dmi/tables/DMI. Only SMBIOS3 is
// supported. See https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.0.0.pdf
// for the specification that was used to implement this.
//
// This is a common location for x86.

struct dmi_info {
    const char *serial;
};

static const int max_strings = 16;

static int index_strings(const uint8_t *data, int max_len, const char **strings, int max_strings)
{
    // Scan the string data and index the strings

    const uint8_t *p = data;
    const uint8_t *lastp = data + max_len;

    int ix = 0;
    while (p < lastp && *p != '\0') {
        // Found a string
        if (ix < max_strings) {
            strings[ix] = (const char *) p;
            ix++;
        }

        // Skip over the string
        while (p < lastp && *p != '\0')
            p++;
        p++;
    }

    // Clear out any missing strings
    while (ix < max_strings)
        strings[ix++] = NULL;

    // Return the amount of data we processed.
    return p - data + 1;
}

static int scan_system_information(const uint8_t *structure, const char **strings, struct dmi_info *result)
{
    uint8_t structure_len = structure[1];

    // SMBIOS 2.0 has the serial number field
    if (structure_len < 0x08)
        return 0;

    uint8_t serial_number_ix = structure[7] - 1;
    if (serial_number_ix < max_strings)
        result->serial = strings[serial_number_ix];

    return 0;
}

static int scan_smbios_structure(const uint8_t *structure, const char **strings, struct dmi_info *result)
{
    uint8_t structure_type = structure[0];

    switch (structure_type) {
        case 1: // System information
            return scan_system_information(structure, strings, result);

        case 0: // BIOS Information
        case 3: // System enclosure
        case 4: // Processor information
        case 7: // Cache information
        case 9: // System slots
        case 16: // Physical memory array
        case 17: // Memory device
        case 19: // Memory array mapped address
        case 32: // System boot
        default:
            // Unimplemented, so skip to the next structure
            return 1;
    }
}

static int scan_smbios_structures(const uint8_t *data, int len, struct dmi_info *result)
{
    memset(result, 0, sizeof(struct dmi_info));

    while (len > 4) {
        const uint8_t *structure = data;
        uint8_t structure_len = data[1];

        // Stop if there's a bad length or a parse error.
        if (structure_len > len)
           break;

        const char *strings[max_strings];

        int all_string_len = index_strings(data + structure_len, len - structure_len, strings, max_strings);

        if (scan_smbios_structure(structure, strings, result) == 0)
            break;

        int total_len = structure_len + all_string_len;

        data += total_len;
        len -= total_len;
    }

    if (result->serial)
        return 1;
    else
        return 0;
}


bool dmi_id(const struct boardid_options *options, char *buffer)
{
    FILE *fp = fopen_helper("/sys/firmware/dmi/tables/DMI", "r");
    if (!fp)
        return false;

    uint8_t data[4096];
    size_t amount_read = fread(data, 1, sizeof(data), fp);
    fclose(fp);

    // See if we can find the serial number in the DMI info
    struct dmi_info result;
    if (!scan_smbios_structures(data, amount_read, &result))
        return false;

    int digits = strlen(result.serial);
    if (digits > MAX_SERIALNUMBER_LEN)
        digits = MAX_SERIALNUMBER_LEN;

    memcpy(buffer, result.serial, digits);
    buffer[digits] = '\0';

    return true;
}
