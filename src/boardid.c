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

#include <getopt.h>
#include <err.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

// The root prefix is used for unit testing so that simulated /proc or /sys
// files can be used.
const char *root_prefix = "";

struct board_id_pair {
    const char **aliases;
    int (*read_id)(const struct id_options *options, char *buffer, int len);
    bool autodetect; // true to attempt autodetection
};

static const char *cpuinfo_aliases[] = {
    "rpi",      "Raspberry Pi (all models)",
    "ev3",      "Lego EV3",
    "chip",     "Next Thing Co - C.H.I.P.",
    "cpuinfo",  "Read /proc/cpuinfo",
    NULL,       NULL
};

static const char *bbb_aliases[] = {
    "bbb",      "Beaglebone Black",
    NULL,       NULL
};

static const char *macaddr_aliases[] = {
    "macaddr",  "Read eth0's MAC address",
    NULL,       NULL
};

static const char *binfile_aliases[] = {
    "binfile",  "Read '-l' bytes from the file '-f' at offset '-k'",
    NULL,       NULL
};

static const char *linkit_aliases[] = {
    "linkit",  "LinkIt Smart (WLAN MAC address)",
    NULL,       NULL
};

static const char *uboot_env_aliases[] = {
    "uboot_env",  "Read a U-Boot environment (file '-f', offset '-k', length '-l') and use the variable '-u'",
    NULL,       NULL
};

static struct board_id_pair boards[] = {
    { cpuinfo_aliases, cpuinfo_id, true },
    { bbb_aliases, beagleboneblack_id, true },
    { macaddr_aliases, macaddr_id, true },
    { linkit_aliases, linkit_id, true },
    { binfile_aliases, binfile_id, false },
    { uboot_env_aliases, uboot_env_id, false},
    { NULL, NULL }
};

static void usage()
{
    printf("boardid prints a platform-specific identifier.\n");
    printf("\n");
    printf("Usage: boardid [OPTION]...\n");
    printf("\n");
    printf("Options:\n");
    printf("  -b <board/method> Use the specified board or detection method for\n");
    printf("                    reading the ID.\n");
    printf("  -f <path>         The file to read for the 'binfile'/'uenv' methods\n");
    printf("  -k <offset>       The offset in bytes for the 'binfile'/`uenv' methods\n");
    printf("  -l <count>        The number of bytes to read for the 'binfile'/'uenv' methods\n");
    printf("  -u <varname>      U-boot environment variable name for the 'uenv' method\n");
    printf("  -n <count>        Print out count characters (least significant ones)\n");
    printf("  -r <prefix>       Root directory prefix (used for unit tests)\n");
    printf("  -v                Print out the program version\n");
    printf("\n");
    printf("'-b' can be specified multiple times to try more than one method.\n");
    printf("\n");
    printf("Supported boards/methods:\n");
    for (struct board_id_pair *b = boards; b->aliases; b++) {
        for (const char **alias = b->aliases; *alias != NULL; alias += 2)
            printf("  %-9s  %s\n", alias[0], alias[1]);
    }
    printf("\n");
}

static struct board_id_pair *find_board(const char *name)
{
    struct board_id_pair *board = boards;
    while (board->aliases) {
        for (const char **alias = board->aliases; *alias != NULL; alias += 2) {
            if (strcmp(name, *alias) == 0)
                return board;
        }
        board++;
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    // Defaults:
    //   Print all digits in the id
    //   Auto-detect the board
    int digits = MAX_SERIALNUMBER_LEN;
    struct id_options options[MAX_STRATEGIES_TO_TRY];
    memset(options, 0, sizeof(options));
    int current_set = -1;

    int opt;
    while ((opt = getopt(argc, argv, "b:f:k:l:n:r:vu:")) != -1) {
        switch (opt) {
        case 'b':
            current_set++;
            if (current_set >= MAX_STRATEGIES_TO_TRY)
                errx(EXIT_FAILURE, "Too many '-b' options");

            options[current_set].name = optarg;
            break;

        case 'f':
            if (current_set < 0)
                errx(EXIT_FAILURE, "Specify '-b' first");
            options[current_set].filename = optarg;
            break;

        case 'k':
            if (current_set < 0)
                errx(EXIT_FAILURE, "Specify '-b' first");
            options[current_set].offset = strtol(optarg, 0, 0);
            break;

        case 'l':
            if (current_set < 0)
                errx(EXIT_FAILURE, "Specify '-b' first");
            options[current_set].size = strtol(optarg, 0, 0);
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

        case 'u':
            if (current_set < 0)
                errx(EXIT_FAILURE, "Specify '-b' first");
            options[current_set].uenv_varname = optarg;
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
    if (current_set >= 0) {
        // Check using user-specified strategies
        for (int i = 0; i <= current_set && !worked; i++) {
            struct board_id_pair *board = find_board(options[i].name);
            if (board == NULL)
                errx(EXIT_FAILURE, "Unsupported strategy '%s'", options[i].name);

            worked = board->read_id(&options[i], serial_number, digits + 1);
        }
    } else {
        // Try each board until one works
        struct board_id_pair *board = boards;
        while (board->read_id && !worked) {
            worked = board->read_id(&options[0], serial_number, digits + 1);
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
