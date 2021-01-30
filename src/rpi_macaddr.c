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

#include "common.h"

// Compute MAC addresses of builtin network interfaces for RPI based on serial number.

#define RPI_OUI {0xB8,0x27,0xEB}
#define RPI4_OUI {0xDC,0xA6,0x32}

#define RPI_WLAN_XOR_MAGIC  0x55
#define RPI_ETH_XOR_MAGIC  0x00

#define MAC_ADDR_NUMBYTES  6
#define MAC_ADDR_OUI_NUMBYTES  3
#define MAC_ADDR_SER_NUMBYTES  3

// Helper function to compute the different variations of eth0/wlan0 and RPI/RPI4
static bool rpi_macaddr_id(const struct boardid_options *options, char *buffer, const uint8_t mac_oui[MAC_ADDR_OUI_NUMBYTES], const uint8_t xor_magic)
{
    // get the serial number first
    // it comes as a HEX string
    char serno_string[MAX_SERIALNUMBER_LEN+1] = "";
    if (!cpuinfo_id(options, serno_string))
        return false;

    size_t serno_len = strnlen(serno_string, sizeof(serno_string));

    // need at least 6 digits of hex serial number (last 3 bytes used for MAC)
    if (serno_len < MAC_ADDR_NUMBYTES)
        return false;

    uint8_t serno_bytes[MAC_ADDR_SER_NUMBYTES] = {0};
    hex_to_bin(serno_string + (serno_len - MAC_ADDR_SER_NUMBYTES*2), MAC_ADDR_SER_NUMBYTES, serno_bytes);


    uint8_t mac_bytes[MAC_ADDR_NUMBYTES] = {0};
    // fill in OUI part of MAC address
    memcpy(mac_bytes, mac_oui, MAC_ADDR_OUI_NUMBYTES);

    // fill in ser number part of MAC address
    for (size_t i = 0; i < sizeof(serno_bytes); ++i) {
        mac_bytes[MAC_ADDR_OUI_NUMBYTES + i] = xor_magic ^ serno_bytes[i];
    }

    char mac_string[MAC_ADDR_NUMBYTES*2+1] = "";
    bin_to_hex(mac_bytes, MAC_ADDR_NUMBYTES, mac_string);

    // Copy the serial number to the output buffer
    int digits = MAC_ADDR_NUMBYTES*2; //
    if (digits > MAX_SERIALNUMBER_LEN)
        digits = MAX_SERIALNUMBER_LEN;

    memcpy(buffer, mac_string, digits);
    buffer[digits] = '\0';

    return true;
}

bool rpi_wlan0_macaddr_id(const struct boardid_options *options, char *buffer)
{
    uint8_t mac_oui[MAC_ADDR_OUI_NUMBYTES] = RPI_OUI;
    return rpi_macaddr_id(options, buffer, mac_oui, RPI_WLAN_XOR_MAGIC);
}

bool rpi_eth0_macaddr_id(const struct boardid_options *options, char *buffer)
{
    uint8_t mac_oui[MAC_ADDR_OUI_NUMBYTES] = RPI_OUI;
    return rpi_macaddr_id(options, buffer, mac_oui, RPI_ETH_XOR_MAGIC);
}

bool rpi4_wlan0_macaddr_id(const struct boardid_options *options, char *buffer)
{
    uint8_t mac_oui[MAC_ADDR_OUI_NUMBYTES] = RPI4_OUI;
    return rpi_macaddr_id(options, buffer, mac_oui, RPI_WLAN_XOR_MAGIC);
}

bool rpi4_eth0_macaddr_id(const struct boardid_options *options, char *buffer)
{
    uint8_t mac_oui[MAC_ADDR_OUI_NUMBYTES] = RPI4_OUI;
    return rpi_macaddr_id(options, buffer, mac_oui, RPI_ETH_XOR_MAGIC);
}
