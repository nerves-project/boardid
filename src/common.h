// SPDX-FileCopyrightText: 2016 Frank Hunleth
// SPDX-FileCopyrightText: 2023 Jon Ringle
//
// SPDX-License-Identifier: Apache-2.0

#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#define MAX_SERIALNUMBER_LEN  32
#define MAX_STRATEGIES_TO_TRY 8
#define MAX_ARGC 64

#ifndef PROGRAM_VERSION
#define PROGRAM_VERSION unknown
#endif

#define xstr(s) str(s)
#define str(s) #s
#define PROGRAM_VERSION_STR xstr(PROGRAM_VERSION)

FILE *fopen_helper(const char *filename, const char *mode);
void bin_to_hex(const uint8_t *input, size_t len, int capital_hex, char *output);
void merge_config(int argc, char *argv[], int *merged_argc, char **merged_argv, int max_args);

struct boardid_options
{
    // Options for all ID queriers
    int digits; // how many digits of the serial number to return

    // Options that depend on the querier
    const char *filename;
    off_t offset;
    size_t size;
    const char *uenv_varname;
    int capital_hex;
    int i2c_address;
};

bool atecc508a_id(const struct boardid_options *options, char *buffer);
bool beagleboneblack_id(const struct boardid_options *options, char *buffer);
bool binfile_id(const struct boardid_options *options, char *buffer);
bool cpuinfo_id(const struct boardid_options *options, char *buffer);
bool device_tree_id(const struct boardid_options *options, char *buffer);
bool dmi_id(const struct boardid_options *options, char *buffer);
bool force_id(const struct boardid_options *options, char *buffer);
bool grisp_id(const struct boardid_options *options, char *buffer);
bool linkit_id(const struct boardid_options *options, char *buffer);
bool macaddr_id(const struct boardid_options *options, char *buffer);
bool nerves_key_id(const struct boardid_options *options, char *buffer);
bool rpi_eth0_macaddr_id(const struct boardid_options *options, char *buffer);
bool rpi_wlan0_macaddr_id(const struct boardid_options *options, char *buffer);
bool script_id(const struct boardid_options * options, char *buffer);
bool uboot_env_id(const struct boardid_options *options, char *buffer);

// The root prefix is used for unit testing so that simulated /proc or /sys
// files can be used.
extern const char *root_prefix;

#endif // COMMON_H
