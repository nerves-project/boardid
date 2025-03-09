// SPDX-FileCopyrightText: 2016 Chris Dutton
// SPDX-FileCopyrightText: 2016 Frank Hunleth
// SPDX-FileCopyrightText: 2021 Dömötör Gulyás
// SPDX-FileCopyrightText: 2023 Jon Ringle
//
// SPDX-License-Identifier: Apache-2.0

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
const char *root_prefix = NULL;

struct board_id_pair {
    const char **aliases;
    bool (*read_id)(const struct boardid_options *options, char *buffer);
    bool autodetect; // true to attempt autodetection
    bool trim_left; // true to trim serial numbers from the left
};

static const char *cpuinfo_aliases[] = {
    "rpi",      "Raspberry Pi (all models)",
    "ev3",      "Lego EV3",
    "chip",     "Next Thing Co - C.H.I.P.",
    "cpuinfo",  "Read /proc/cpuinfo",
    NULL,       NULL
};

static const char *rpi_eth0_aliases[] = {
    "rpi_eth0", "Raspberry Pi eth0 MAC address",
    NULL,       NULL
};

static const char *rpi_wlan0_aliases[] = {
    "rpi_wlan0", "Raspberry Pi wlan0 MAC address",
    NULL,       NULL
};

static const char *device_tree_aliases[] = {
    "jetson",   "nVidia Jetson",
    "device_tree",  "Read /proc/device-tree/serial-number",
    NULL,       NULL
};

static const char *bbb_aliases[] = {
    "bbb",      "Beaglebone and BeaglePlay",
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

static const char *atecc508a_aliases[] = {
    "atecc508a",  "Read an ATECC508A (I2C device '-f', I2C address '-a')",
    NULL,       NULL
};

static const char *nerves_key_aliases[] = {
    "nerves_key",  "Read a NervesKey (I2C device '-f', I2C address '-a')",
    NULL,       NULL
};

static const char *dmi_aliases[] = {
    "dmi",  "Read the system ID out of the SMBIOS/DMI",
    NULL,       NULL
};

static const char *grisp_aliases[] = {
    "grisp",      "GRiSP",
    NULL,       NULL
};

static const char *force_aliases[] = {
    "force",  "Force the ID (Specify ID with '-f')",
    NULL,       NULL
};

static const char *script_aliases[] = {
    "script", "Run a script to get the ID (Specify script with '-f')",
    NULL,       NULL
};

// aliases, read_id, autodetect, trim_left
static const struct board_id_pair boards[] = {
    { atecc508a_aliases, atecc508a_id, false, false },
    { bbb_aliases, beagleboneblack_id, true, false },
    { binfile_aliases, binfile_id, false, false },
    { cpuinfo_aliases, cpuinfo_id, true, false },
    { device_tree_aliases, device_tree_id, true, false },
    { dmi_aliases, dmi_id, true, false },
    { force_aliases, force_id, false, true },
    { grisp_aliases, grisp_id, true, false },
    { linkit_aliases, linkit_id, true, false },
    { macaddr_aliases, macaddr_id, true, false },
    { nerves_key_aliases, nerves_key_id, false, false },
    { rpi_eth0_aliases, rpi_eth0_macaddr_id, false, false },
    { rpi_wlan0_aliases, rpi_wlan0_macaddr_id, false, false },
    { script_aliases, script_id, false, false},
    { uboot_env_aliases, uboot_env_id, false, true},
    { NULL, NULL, false, false }
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
    printf("  -f <path>         The file to read for the methods requiring files\n");
    printf("  -k <offset>       The offset in bytes for the 'binfile'/`uenv' methods\n");
    printf("  -l <count>        The number of bytes to read for the 'binfile'/'uenv' methods\n");
    printf("  -u <varname>      U-boot environment variable name for the 'uenv' method\n");
    printf("  -n <count>        Print out count characters (least significant ones)\n");
    printf("  -p <string>       Prefix an ID with the specific string\n");
    printf("  -r <prefix>       Root directory prefix (used for unit tests)\n");
    printf("  -a <i2c address>  I2C bus address\n");
    printf("  -X                Print capital hex digits for `binfile`/`atecc508a` methods\n");
    printf("  -v                Print out the program version\n");
    printf("\n");
    printf("'-b' can be specified multiple times to try more than one method.\n");
    printf("\n");
    printf("Supported boards/methods:\n");
    for (const struct board_id_pair *b = boards; b->aliases; b++) {
        for (const char **alias = b->aliases; *alias != NULL; alias += 2)
            printf("  %-9s  %s\n", alias[0], alias[1]);
    }
    printf("\n");
}

static const struct board_id_pair *find_board(const char *name)
{
    const struct board_id_pair *board = boards;
    while (board->aliases) {
        for (const char **alias = board->aliases; *alias != NULL; alias += 2) {
            if (strcmp(name, *alias) == 0)
                return board;
        }
        board++;
    }
    warnx("Couldn't find ID querier '%s'", name);
    return NULL;
}

static void right_trim_serial_number(char *serial_number, int desired_digits)
{
    // If the user doesn't want all the digits, return the rightmost
    // ones. This heuristic tends to be more desirable when it's needed
    // since leftmost digits frequently change less.
    int num_digits = strlen(serial_number);
    if (desired_digits < num_digits) {
        int offset = num_digits - desired_digits;
        memmove(serial_number, serial_number + offset, desired_digits);
        serial_number[desired_digits] = '\0';
    }
}

static void left_trim_serial_number(char *serial_number, int desired_digits)
{
    // Some options are left trimmed since the original implementation worked
    // that way and too much time has passed to comfortably change it without
    // risking breakage in the field.
    serial_number[desired_digits] = '\0';
}

static void trim_serial_number(char *serial_number, int desired_digits, bool trim_left)
{
    if (trim_left)
        left_trim_serial_number(serial_number, desired_digits);
    else
        right_trim_serial_number(serial_number, desired_digits);
}

/**
 * Try autodetecting the ID
 *
 * The serial_number buffer must be MAX_SERIALNUMBER_LEN + 1 or
 * larger.
 *
 * desired_digits is a maximum number of digits to return.
 *
 * Returns true if a serial number was found
 * Returns false if couldn't be autodetected
 */
bool boardid_autodetect(int desired_digits, char *serial_number)
{
    struct boardid_options null_options;
    memset(&null_options, 0, sizeof(null_options));

    // Try each board until one works
    const struct board_id_pair *board = boards;
    while (board->read_id) {
        if (board->autodetect) {
            memset(serial_number, 0, MAX_SERIALNUMBER_LEN + 1);
            bool worked = board->read_id(&null_options, serial_number);
            if (worked) {
                trim_serial_number(serial_number, desired_digits, board->trim_left);
                return true;
            }
        }
        board++;
    }

    return false;
}

/**
 * Read a board's serial number
 *
 * A string identifying the serial number querier to use. Depending on
 * the querier, some or all of the fields in the boardid_options struct
 * will need to be filled out. See the struct and the querier docs for
 * details.
 *
 * The serial_number buffer must be MAX_SERIALNUMBER_LEN + 1 or
 * larger.
 *
 * Returns true if a serial number was found
 * Returns false if not
 */
bool boardid_read(const char *querier, const struct boardid_options *options, char *serial_number)
{
    memset(serial_number, 0, MAX_SERIALNUMBER_LEN + 1);
    const struct board_id_pair *board = find_board(querier);
    if (board && board->read_id(options, serial_number)) {
        trim_serial_number(serial_number, options->digits, board->trim_left);
        return true;
    } else {
        return false;
    }
}

struct cmdline_option {
    const char *querier_name;
    const char *prefix;
    struct boardid_options id_options;
};

int main(int argc, char *argv[])
{
    // Defaults:
    //   Print all digits in the id
    //   Auto-detect the board
    int default_digits = MAX_SERIALNUMBER_LEN;
    struct cmdline_option options[MAX_STRATEGIES_TO_TRY];
    memset(options, 0, sizeof(options));
    int current_set = -1;

    if (argc >= 3 && strcmp(argv[1], "-r") == 0)
        root_prefix = argv[2];

    int merged_argc;
    char *merged_argv[MAX_ARGC];
    merge_config(argc, argv, &merged_argc, merged_argv, MAX_ARGC);

    int opt;
    while ((opt = getopt(merged_argc, merged_argv, "a:b:f:k:l:n:p:r:vu:X?")) != -1) {
        switch (opt) {
        case 'b':
            current_set++;
            if (current_set >= MAX_STRATEGIES_TO_TRY)
                errx(EXIT_FAILURE, "Too many '-b' options");

            options[current_set].querier_name = optarg;
            options[current_set].id_options.digits = default_digits;

            break;

        case 'f':
            if (current_set < 0)
                errx(EXIT_FAILURE, "Specify '-b' first");
            options[current_set].id_options.filename = optarg;
            break;

        case 'k':
            if (current_set < 0)
                errx(EXIT_FAILURE, "Specify '-b' first");
            options[current_set].id_options.offset = strtol(optarg, 0, 0);
            break;

        case 'l':
            if (current_set < 0)
                errx(EXIT_FAILURE, "Specify '-b' first");
            options[current_set].id_options.size = strtol(optarg, 0, 0);
            break;

        case 'n':
        {
            int digits = strtol(optarg, 0, 0);
            if (digits < 1)
                digits = 1;
            else if (digits > MAX_SERIALNUMBER_LEN)
                digits = MAX_SERIALNUMBER_LEN;

            if (current_set < 0)
                default_digits = digits;
            else
                options[current_set].id_options.digits = digits;
            break;
        }

        case 'p':
            if (current_set < 0)
                errx(EXIT_FAILURE, "Specify '-b' first");
            options[current_set].prefix = optarg;
            break;

        case 'r':
            root_prefix = optarg;
            break;

        case 'a':
            if (current_set < 0)
                errx(EXIT_FAILURE, "Specify '-b' first");
            options[current_set].id_options.i2c_address = strtol(optarg, 0, 0);
            break;

        case 'v':
            printf("%s\n", PROGRAM_VERSION_STR);
            exit(EXIT_SUCCESS);

        case 'u':
            if (current_set < 0)
                errx(EXIT_FAILURE, "Specify '-b' first");
            options[current_set].id_options.uenv_varname = optarg;
            break;

        case 'X':
            if (current_set < 0)
                errx(EXIT_FAILURE, "Specify '-b' first");
            options[current_set].id_options.capital_hex = 1;
            break;

        case '?':
        default:
            usage();
            exit(EXIT_FAILURE);
        }
    }

    if (optind < merged_argc) {
        // Support deprecated mode of passing the digit count via a positional argument
        default_digits = strtol(argv[optind], 0, 0);
        if (default_digits < 1)
            default_digits = 1;
        else if (default_digits > MAX_SERIALNUMBER_LEN)
            default_digits = MAX_SERIALNUMBER_LEN;

        if (current_set >= 0)
            options[current_set].id_options.digits = default_digits;
    }

    char serial_number[MAX_SERIALNUMBER_LEN + 1];
    bool worked = false;
    const char *prefix = "";
    if (current_set >= 0) {
        // Check using user-specified strategies
        for (int i = 0; i <= current_set; i++) {
            if (boardid_read(options[i].querier_name, &options[i].id_options, serial_number)) {
                if (options[i].prefix)
                    prefix = options[i].prefix;
                worked = true;
                break;
            }
        }
    } else {
        worked = boardid_autodetect(default_digits, serial_number);
    }

    if (worked) {
        // Success
        printf("%s%s\n", prefix, serial_number);
    } else {
        // Failure: print all zeros
        int to_print = (current_set >= 0) ? options[current_set].id_options.digits : default_digits;
        for (int i = 0; i < to_print; i++)
            printf("0");
        printf("\n");
    }

    exit(EXIT_SUCCESS);
}
