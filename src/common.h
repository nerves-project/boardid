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
void bin_to_hex(const uint8_t *input, size_t len, char *output);
void merge_config(int argc, char *argv[], int *merged_argc, char **merged_argv, int max_args);

struct boardid_options
{
    // Options for all ID queriers
    int digits; // how many digits of the serial number to return

    // Options that depend on the querier
    const char *filename;
    int offset;
    int size;
    const char *uenv_varname;
};

bool cpuinfo_id(const struct boardid_options *options, char *buffer);
bool device_tree_id(const struct boardid_options *options, char *buffer);
bool macaddr_id(const struct boardid_options *options, char *buffer);
bool beagleboneblack_id(const struct boardid_options *options, char *buffer);
bool linkit_id(const struct boardid_options *options, char *buffer);
bool binfile_id(const struct boardid_options *options, char *buffer);
bool uboot_env_id(const struct boardid_options *options, char *buffer);
bool atecc508a_id(const struct boardid_options *options, char *buffer);
bool nerves_key_id(const struct boardid_options *options, char *buffer);
bool dmi_id(const struct boardid_options *options, char *buffer);
bool force_id(const struct boardid_options *options, char *buffer);

// The root prefix is used for unit testing so that simulated /proc or /sys
// files can be used.
extern const char *root_prefix;

#endif // COMMON_H
