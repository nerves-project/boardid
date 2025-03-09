// SPDX-FileCopyrightText: 2018 Frank Hunleth
//
// SPDX-License-Identifier: Apache-2.0

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
    // commandline arguments and then boardid.config
    // arguments.
    // This way, the commandline takes precedence.

    if (argc > max_args)
        argc = max_args;
    memcpy(&merged_argv[0], &argv[0], argc * sizeof(char**));
    *merged_argc = argc;

    *merged_argc += load_config("/etc/boardid.config",
                                &merged_argv[argc],
                                max_args - argc);

    if (*merged_argc == max_args)
        warnx("Dropping excess commandline/config arguments. Max is %d.", max_args);
}


