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

#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

// The root prefix is used for unit testing so that similated /proc or /sys
// files can be used.
const char *root_prefix = "";

struct board_id_pair {
    const char *name;
    int (*read_id)(char *buffer, int len);
};

static struct board_id_pair boards[] = {
    { "rpi", raspberrypi_id },
    { "bbb", beagleboneblack_id },
    { NULL, NULL }
};

static void usage()
{
    printf("boardid prints a platform-specific unique identifier.\n");
    printf("\n");
    printf("Usage: boardid [OPTION]...\n");
    printf("\n");
    printf("Options:\n");
    printf("  -b <platform>   Use the board id method for the specified platform\n");
    printf("  -n <count>      Print out count characters (least significant ones)\n");
    printf("  -r <prefix>     Root directory prefix (used for unit tests)\n");
    printf("  -v              Print out the program version\n");
    printf("\n");
    printf("Supported boards:\n");
    for (struct board_id_pair *b = boards; b->name; b++)
        printf("   %s\n", b->name);
    printf("\n");
}

int main(int argc, char *argv[])
{
    // Defaults:
    //   Print all digits in the id
    //   Auto-detect the board
    int digits = MAX_SERIALNUMBER_LEN;
    const char *name = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "b:n:r:v")) != -1) {
        switch (opt) {
        case 'b':
            name = optarg;
            break;

        case 'n':
            digits = strtol(optarg, 0, 0);
            break;

        case 'r':
            root_prefix = optarg;
            break;

        case 'v':
            printf("%s\n", PROGRAM_VERSION_STR);
            exit(EXIT_SUCCESS);
            break;

        default:
            usage();
            exit(EXIT_FAILURE);
            break;
        }
    }

    if (optind < argc) {
        // Support deprecated mode of passing the digit count via a positional argument
        digits = strtol(argv[optind], 0, 0);
    }

    if (digits < 1)
        digits = 1;
    else if (digits > MAX_SERIALNUMBER_LEN)
        digits = MAX_SERIALNUMBER_LEN;

    char serial_number[MAX_SERIALNUMBER_LEN + 1];
    int worked = 0;
    if (name) {
        // Only check the specified board for the serial number
        struct board_id_pair *board = boards;
        while (board->name) {
            if (strcmp(name, board->name) == 0)
                break;
            board++;
        }
        if (board->name == NULL)
            errx(EXIT_FAILURE, "Unsupport board '%s'", name);

        worked = board->read_id(serial_number, digits + 1);
    } else {
        // Try each board until one works
        struct board_id_pair *board = boards;
        while (board->name && !worked) {
            worked = board->read_id(serial_number, digits + 1);
            board++;
        }
    }

    if (worked) {
        // Success
        printf("%s\n", serial_number);
    } else {
        // Failure: print all zeros
        for (int i = 0; i < digits; i++)
            printf("0");
        printf("\n");
    }

    exit(EXIT_SUCCESS);
}
