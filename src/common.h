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

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

#define MAX_SERIALNUMBER_LEN  32
#define MAX_STRATEGIES_TO_TRY 8

#ifndef PROGRAM_VERSION
#define PROGRAM_VERSION unknown
#endif

#define xstr(s) str(s)
#define str(s) #s
#define PROGRAM_VERSION_STR xstr(PROGRAM_VERSION)

FILE *fopen_helper(const char *filename, const char *mode);

struct id_options
{
    const char *name;
    int digits; // how many digits of the serial number to return

    const char *filename;
    int offset;
    int size;
    const char *uenv_varname;
};

int cpuinfo_id(const struct id_options *options, char *buffer, int len);
int macaddr_id(const struct id_options *options, char *buffer, int len);
int beagleboneblack_id(const struct id_options *options, char *buffer, int len);
int linkit_id(const struct id_options *options, char *buffer, int len);
int binfile_id(const struct id_options *options, char *buffer, int len);
int uboot_env_id(const struct id_options *options, char *buffer, int len);

#endif // COMMON_H
