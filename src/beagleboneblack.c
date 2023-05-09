// SPDX-FileCopyrightText: 2020 Nerves Project Developers
//
// SPDX-License-Identifier: Apache-2.0

#include <string.h>
#include <stdlib.h>

#include "common.h"

static void extract_bbb_serial(const unsigned char *data, char *buffer)
{
    // BBB EEPROM Layout
    // See the SRM
    //
    // Offset Size Contents
    // 0      4    Header (AA,55,33,EE)
    // 4      8    Board name in ASCII
    // 12     4    Version in ASCII
    // 16     12   Serial number in ASCII
    // 28     32   Configuration setup

    const int digits = 12;
    memcpy(buffer, &data[16], digits);
    buffer[digits] = 0;
}

static void extract_beagleplay_serial(const unsigned char *data, char *buffer)
{
    // BeaglePlay EEPROM Layout
    // See https://git.beagleboard.org/beagleplay/beagleplay/-/blob/main/EEPROM.md
    //
    // Multibyte integers are little endian
    //
    // Offset Size Contents
    // 0      4    Header (AA,55,33,EE)
    // 4      1    Type (1)
    // 5      2    Payload size (0x37)
    // 7      1    BRD_INFO (0x10)
    // 8      2    0x2e
    // 10     16   Board_Name (BEAGLEPLAY-A0-\0\0)
    // 26     2    Design_Rev (0x02)
    // 28     4    PROC_Nbr
    // ...
    // 40     2    VendorID (ASCII "64")
    // 42     2    Build_Week (ASCII)
    // 44     2    Build_Year (ASCII)
    // 46     6    Board_ID (ASCII)
    // 52     4    Serial_Nbr (ASCII "SSSS")
    //
    // The serial number is set to all S's, so use the Board_ID.
    // The Board_ID is the last 6 digits on the QRCode sticker.
    // E.g., BPVA2022510000123 where 2022 is the Build_Year and 51 is the
    // Build_Week.

    const int digits = 6;
    memcpy(buffer, &data[46], digits);
    buffer[digits] = 0;
}

// Read the serial number from the Beaglebone's EEPROM
bool beagleboneblack_id(const struct boardid_options *options, char *buffer)
{
    // Try both the Linux 3.8 and 4.1 EEPROM locations
    FILE *fp = fopen_helper("/sys/bus/i2c/devices/0-0050/eeprom", "r");
    if (!fp)
        fp = fopen_helper("/sys/bus/i2c/devices/0-0050/at24-0/nvmem", "r");

    if (!fp)
        return 0;

    unsigned char data[56];
    if (fread(data, 1, sizeof(data), fp) != sizeof(data)) {
        fclose(fp);
        return false;
    }

    fclose(fp);

    // Verify that the EEPROM was programmed
    if (data[0] != 0xaa ||
            data[1] != 0x55 ||
            data[2] != 0x33 ||
            data[3] != 0xee)
        return false;

    // BeaglePlay has a non-ASCII 1 at offset 4 and follow it with the data
    // size. Beagleboards should have ASCII letters there.
    if (data[4] == 0x01 && data[5] == 0x37 && data[6] == 0)
        extract_beagleplay_serial(data, buffer);
    else
        extract_bbb_serial(data, buffer);

    return true;
}
