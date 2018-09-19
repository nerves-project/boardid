/*
   Copyright 2016 Frank Hunleth

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <string.h>
#include <stdlib.h>

#include "common.h"

#define MAX_BINFILE_ID_LEN 64

int binfile_id(const struct id_options *options, char *buffer, int len)
{
    FILE *fp = fopen_helper(options->filename, "r");
    if (!fp)
        return 0;

    if (fseek(fp, options->offset, SEEK_SET) < 0) {
        fclose(fp);
        return 0;
    }

    int idlen = options->size;
    if (idlen > MAX_BINFILE_ID_LEN)
        idlen = MAX_BINFILE_ID_LEN;
    uint8_t data[MAX_BINFILE_ID_LEN];

    if (fread(data, 1, idlen, fp) != idlen) {
        fclose(fp);
        return 0;
    }

    fclose(fp);

    char hexdata[MAX_BINFILE_ID_LEN * 2 + 1];
    bin_to_hex(data, idlen, hexdata);

    // Return the least significant digits of the serial number
    // if not returning all of them.
    int maxdigits = idlen * 2;
    int digits = len - 1;
    if (digits > maxdigits)
        digits = maxdigits;
    memcpy(buffer, &hexdata[maxdigits - digits], digits);
    buffer[digits] = 0;
    return 1;
}

int linkit_id(const struct id_options *options, char *buffer, int len)
{
    // Use the LinkIt Smart's WLAN MAC address for the unique ID.
    struct id_options linkit_options;
    linkit_options.filename = "/dev/mtdblock2";
    linkit_options.size = 6;
    linkit_options.offset = 0x4;

    return binfile_id(&linkit_options, buffer, len);
}
