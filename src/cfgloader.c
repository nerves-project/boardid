/*
   Copyright 2018 Frank Hunleth

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

#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

static int parse_config_line(char *line, char **argv, int max_args)
{
    int argc = 0;
    char *c = line;
    char *token = NULL;
#define STATE_SPACE 0
#define STATE_TOKEN 1
#define STATE_QUOTED_TOKEN 2
    int state = STATE_SPACE;
    while (*c != '\0') {
        switch (state) {
        case STATE_SPACE:
            if (*c == '#')
                return argc;
            else if (isspace(*c))
                break;
            else if (*c == '"') {
                token = c + 1;
                state = STATE_QUOTED_TOKEN;
            } else {
                token = c;
                state = STATE_TOKEN;
            }
            break;
        case STATE_TOKEN:
            if (*c == '#' || isspace(*c)) {
                *argv = strndup(token, c - token);
                argv++;
                argc++;
                token = NULL;

                if (*c == '#' || argc == max_args)
                    return argc;

                state = STATE_SPACE;
            }
            break;
        case STATE_QUOTED_TOKEN:
            if (*c == '"') {
                *argv = strndup(token, c - token);
                argv++;
                argc++;
                token = NULL;
                state = STATE_SPACE;

                if (argc == max_args)
                    return argc;
            }
            break;
        }
        c++;
    }

    if (token) {
        *argv = strndup(token, c - token);
        argc++;
    }

    return argc;
}

// This is a very simple config file parser that extracts
// commandline arguments from the specified file.
static int load_config(const char *filename,
                       char **argv,
                       int max_args)
{
    FILE *fp = fopen_helper(filename, "r");
    if (!fp)
        return 0;

    int argc = 0;
    char line[128];
    while (fgets(line, sizeof(line), fp) && argc < max_args) {
        int new_args = parse_config_line(line, argv, max_args - argc);
        argc += new_args;
        argv += new_args;
    }

    fclose(fp);
    return argc;
}

void merge_config(int argc, char *argv[], int *merged_argc, char **merged_argv, int max_args)
{
    // When merging, argv[0] is first, then the
    // arguments from boardid.config and then any
    // additional arguments from the commandline.
    // This way, the commandline takes precedence.
    *merged_argc = 1;
    merged_argv[0] = argv[0];

    *merged_argc += load_config("/etc/boardid.config",
                                &merged_argv[1],
                                max_args - argc);

    if (*merged_argc + argc - 1 > max_args) {
        warn("Too many arguments specified between the config file and commandline. Dropping some.");
        argc = max_args - *merged_argc + 1;
    }

    memcpy(&merged_argv[*merged_argc], &argv[1], (argc - 1) * sizeof(char**));
    *merged_argc += argc - 1;
}


