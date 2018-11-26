/*
 * Copyright 2018 Frank Hunleth
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "common.h"

#include <err.h>
#include <string.h>

int force_id(const struct id_options *options, char *buffer, int len)
{
    if (!options->filename) {
        warnx("specify -f to force an ID");
        return 0;
    }

    strncpy(buffer, options->filename, len - 1);
    buffer[len - 1] = '\0';

    if (buffer[0] == '\0')
        return 0;
    else
        return 1;
}
