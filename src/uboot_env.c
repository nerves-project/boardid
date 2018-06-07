/*
 * Copyright 2017 Frank Hunleth
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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

static int uboot_env_read(const char *uenv, size_t size, const char *varname, char *varvalue, int varvalue_len)
{
    uint32_t expected_crc32 = ((uint8_t) uenv[0] | ((uint8_t) uenv[1] << 8) | ((uint8_t) uenv[2] << 16) | ((uint8_t) uenv[3] << 24));
    uint32_t actual_crc32 = crc32buf(uenv + 4, size - 4);
    if (expected_crc32 != actual_crc32) {
        warnx("U-boot environment CRC32 mismatch (expected 0x%08x; got 0x%08x)", expected_crc32, actual_crc32);
        return 0;
    }

    const char *end = uenv + size;
    const char *name = uenv + 4;
    while (name != end && *name != '\0') {
        const char *endname = name + 1;
        for (;;) {
            if (endname == end || *endname == '\0') {
                warnx("Invalid U-boot environment");
                return 0;
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
                return 0;
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

            return 1;
        }

        name = endvalue + 1;
    }

    return 0;
}

static void find_uboot_env(const struct id_options *options, char *filename, int *offset, int *size)
{
    // Default to the input options
    *offset = options->offset;
    *size = options->size;
    *filename = '\0';

    // If the user specified a filename then use it
    if (options->filename) {
        strcpy(filename, options->filename);
        return;
    }

    // Try to read the options from '/etc/fw_env.config'
    FILE *fp = fopen_helper("/etc/fw_env.config", "r");
    if (!fp)
        return;

    while (!feof(fp)) {
        // fw_env.conf has comments, whitespace and lines like follows:
        // # Device name   Device offset   Env. size       Flash sector size       Number of sectors
        // /dev/mmcblk0    0x000000        0x2000          0x200                   16

        int sector_size;
        int num_sectors;
        int num = fscanf(fp, "%255s %i %i %i %i", filename, offset, size, &sector_size, &num_sectors);

        // Check for a match that's not a comment
        if (num == 5 && filename[0] != '#')
            break;
    }
    fclose(fp);
}

int uboot_env_id(const struct id_options *options, char *buffer, int len)
{
    char filename[256];
    int offset;
    int size;
    int rc = 0;

    find_uboot_env(options, filename, &offset, &size);

    FILE *fp = fopen_helper(filename, "rb");
    if (!fp)
        return 0;

    char *uenv = malloc(size);
    if (!uenv)
        goto cleanup;

    if (fseek(fp, offset, SEEK_SET) < 0) {
        warn("seek failed on %s", filename);
        goto cleanup;
    }

    if (fread(uenv, 1, size, fp) != size) {
        warn("fread failed on %s", filename);
        goto cleanup;
    }

    rc = uboot_env_read(uenv, size, options->uenv_varname, buffer, len);

cleanup:
    if (uenv)
        free(uenv);
    fclose(fp);
    return rc;
}
