// SPDX-FileCopyrightText: 2016 Frank Hunleth
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"

static void remove_char(char *str, char c);

// Read the MAC Address from /sys/class/net/eth0/address and use
// it as a unique ID after removing the colons..

bool macaddr_id(const struct boardid_options *options, char *buffer)
{
    FILE *fp = fopen_helper("/sys/class/net/eth0/address", "r");
    if (!fp)
        return false;

    char line[256];
    char *rc = fgets(line, sizeof(line), fp);
    fclose(fp);

    // MAC addresses are always 6 bytes represented in hex
    // with each byte separated with a colon. That is, 12 hex
    // characters and 5 colons, so 17 characters.
    if (!rc || strlen(line) < 17)
       return false;

    // Remove the colons
    remove_char(line, ':');

    // Trim any trailing whitespace
    line[12] = '\0';

    strcpy(buffer, line);

    return true;
}

void remove_char(char *str, char c)
{
    char *dest = str;
    char *src = str;
    while (*src) {
        if (*src != c)
            *dest++ = *src;
        src++;
    }
    *dest = '\0';
}


