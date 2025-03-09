// SPDX-FileCopyrightText: 2018 Frank Hunleth
//
// SPDX-License-Identifier: Apache-2.0

#include "common.h"
#include "crc32.h"

#include <err.h>
#include <stdlib.h>
#include <string.h>

// This code was taken and modified from `fwup` which has the following note:
//
// Licensing note: U-boot is licensed under the GPL which is incompatible with
//                 fwup's Apache 2.0 license. Please don't copy code from
//                 U-boot especially since the U-boot environment data
//                 structure is simple enough to reverse engineer by playing
//                 with mkenvimage.
//

struct uboot_env_location {
    char filename[256];
    int offset;
    int size;
    bool redundant;
};

static int uboot_env_read(const char *uenv,
                           size_t size,
                           bool redundant,
                           const char *varname,
                           char *varvalue,
                           int varvalue_len)
{
    size_t data_offset = redundant ? 5 : 4;
    uint32_t expected_crc32 = ((uint8_t) uenv[0] | ((uint8_t) uenv[1] << 8) | ((uint8_t) uenv[2] << 16) | ((uint8_t) uenv[3] << 24));
    uint32_t actual_crc32 = crc32buf(uenv + data_offset, size - data_offset);
    if (expected_crc32 != actual_crc32) {
        warnx("U-boot environment CRC32 mismatch (expected 0x%08x; got 0x%08x)", expected_crc32, actual_crc32);
        return -1;
    }

    const char *end = uenv + size;
    const char *name = uenv + data_offset;
    while (name != end && *name != '\0') {
        const char *endname = name + 1;
        for (;;) {
            if (endname == end || *endname == '\0') {
                warnx("Invalid U-boot environment");
                return -1;
            }

            if (*endname == '=')
                break;

            endname++;
        }

        const char *value = endname + 1;
        const char *endvalue = value;
        for (;;) {
            if (endvalue == end) {
                warnx("Invalid U-boot environment");
                return -1;
            }

            if (*endvalue == '\0')
                break;

            endvalue++;
        }

        if (strncmp(varname, name, endname - name) == 0) {
            int max_len = endvalue - value;
            if (max_len >= varvalue_len)
                max_len = varvalue_len - 1;
            strncpy(varvalue, value, max_len);
            varvalue[max_len] = 0;

            // If redundant, return the generation number
            return redundant ? (unsigned char) uenv[4] : 0;
        }

        name = endvalue + 1;
    }

    return -1;
}

static int find_uboot_env(const struct boardid_options *options,
                          struct uboot_env_location *locations)
{
    // Default to no locations
    memset(locations, 0, 2 * sizeof(struct uboot_env_location));

    // If the user specified a filename then use it
    if (options->filename) {
        locations->offset = options->offset;
        locations->size = options->size;
        strcpy(locations->filename, options->filename);
        return 1;
    }

    // Try to read the options from '/etc/fw_env.config'
    FILE *fp = fopen_helper("/etc/fw_env.config", "r");
    if (!fp)
        return 0;

    int i = 0;
    while (i <= 2 && !feof(fp)) {
        // fw_env.config has comments, whitespace and lines like follows:
        // # Device name   Device offset   Env. size       Flash sector size       Number of sectors
        // /dev/mmcblk0    0x000000        0x2000          0x200                   16
        char line[256];
        if (fgets(line, sizeof(line), fp) == NULL)
            break;

        int num = sscanf(line, "%255s %i %i",
            locations[i].filename,
            &locations[i].offset,
            &locations[i].size);
        locations[i].redundant = false;

        // Check for a match that's not a comment
        if (num == 3 && locations[i].filename[0] != '#') {
            i++;
        }
    }
    fclose(fp);

    if (i == 2) {
        locations[0].redundant = locations[1].redundant = true;
    }

    return i;
}

static int uboot_read_one_env(const struct uboot_env_location *location,
                              const char *varname,
                              char *buffer,
                              int len)
{
    int rc = -1;

    FILE *fp = fopen_helper(location->filename, "rb");
    if (!fp)
        return -1;

    char *uenv = malloc(location->size);
    if (!uenv)
        goto cleanup;

    if (fseek(fp, location->offset, SEEK_SET) < 0) {
        warn("seek failed on %s", location->filename);
        goto cleanup;
    }

    if (fread(uenv, 1, location->size, fp) != location->size) {
        warn("fread failed on %s", location->filename);
        goto cleanup;
    }

    rc = uboot_env_read(uenv, location->size, location->redundant, varname, buffer, MAX_SERIALNUMBER_LEN + 1);

    // Check for empty string
    if (rc >= 0 && buffer[0] == '\0')
        rc = -1;

cleanup:
    if (uenv)
        free(uenv);
    fclose(fp);
    return rc;
}

bool uboot_env_id(const struct boardid_options *options, char *buffer)
{
    struct uboot_env_location locations[2];

    int count = find_uboot_env(options, locations);
    if (count <= 0)
        return false;

    int generation = uboot_read_one_env(&locations[0], options->uenv_varname, buffer, MAX_SERIALNUMBER_LEN + 1);
    if (count == 2) {
        // Check if the redundant one is better
        char tmp[MAX_SERIALNUMBER_LEN + 1];
        int generation2 =
            uboot_read_one_env(&locations[1], options->uenv_varname, tmp, MAX_SERIALNUMBER_LEN + 1);

        if (generation < 0 && generation2 >= 0) {
            memcpy(buffer, tmp, sizeof(tmp));
            generation = generation2;
        } else if (generation >= 0 && generation2 >= 0) {
            uint8_t gen1 = (uint8_t) generation;
            uint8_t gen2 = (uint8_t) generation2;
            uint8_t delta = gen2 - gen1;
            if (delta > 0 && delta < 127) {
                memcpy(buffer, tmp, sizeof(tmp));
                generation = generation2;
            }
        }
    }
    return generation >= 0;
}
