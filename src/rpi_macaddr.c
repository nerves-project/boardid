/*
   Copyright 2020 Nerves Project Developers

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <byteswap.h>

#include "common.h"

// Read MAC addresses of wlan0/eth0 via vcmailmox from VC firmware.


#define RPI_WLAN_XOR_MAGIC  0x555555
#define RPI_ETH_XOR_MAGIC   0x000000

#define RPI4_WLAN_ADD_MAGIC 2
#define RPI4_ETH_ADD_MAGIC  0

// XOR and ADD magic numbers only affect the last 3 bytes of the MAC address, not the first 3 OUI bytes
// return true when the MAC could be read from vcmailbox, false otherwise
static bool rpi_macaddr_vc(char* dst, uint32_t xor_magic, uint32_t add_magic)
{
    bool success = false;
    if (!dst)
        return success;

    FILE* vc_file = popen("vcmailbox 0x00010003 6 6 0 0", "r");
    if (!vc_file)
        return success;

    char* lineptr = NULL;
    size_t n = 0;
    ssize_t line_len = getline(&lineptr, &n, vc_file);

    if (line_len > 0) {
        unsigned int  mac0 = 0, mac1 = 0;
        if (2 == sscanf(lineptr, "0x00000020 0x80000000 0x00010003 0x00000006 0x80000006 0x%8x 0x%8x 0x00000000", &mac0, &mac1)) {
            if ((mac0 != 0) || (mac1 != 0)) {
                // read parts are little endian, swap for easier printf
                uint64_t macaddr_le = ((uint64_t)(mac1 & 0xFFFF) << 32) | mac0;
                uint64_t macaddr = bswap_64(macaddr_le) >> 16;

                // apply magic numbers
                macaddr = macaddr ^ (xor_magic & 0xFFFFFF);
                // make sure the additive magic number does not affect the OUI part of the MAC
                macaddr = (macaddr & 0xFFFFFF000000) | ((macaddr + add_magic) & 0xFFFFFF);

                snprintf(dst, MAX_SERIALNUMBER_LEN, "%06llx", macaddr);
                success = true;
            }
        }
    }

    // popen() malloc'd lineptr buffer, we have to free it after we're done with it
    if (lineptr)
        free(lineptr);

    pclose(vc_file);

    return success;
}


bool rpi_wlan0_macaddr_id(const struct boardid_options *options, char *buffer)
{
    return rpi_macaddr_vc(buffer, RPI_WLAN_XOR_MAGIC, 0);
}


bool rpi_eth0_macaddr_id(const struct boardid_options *options, char *buffer)
{
    return rpi_macaddr_vc(buffer, RPI_ETH_XOR_MAGIC, 0);
}


bool rpi4_wlan0_macaddr_id(const struct boardid_options *options, char *buffer)
{
    return rpi_macaddr_vc(buffer, 0, RPI4_WLAN_ADD_MAGIC);
}


bool rpi4_eth0_macaddr_id(const struct boardid_options *options, char *buffer)
{
    return rpi_macaddr_vc(buffer, 0, RPI4_ETH_ADD_MAGIC);
}


