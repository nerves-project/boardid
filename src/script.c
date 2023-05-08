// SPDX-FileCopyrightText: 2020 Nerves Project Developers
//
// SPDX-License-Identifier: Apache-2.0

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "common.h"

bool script_id(const struct boardid_options *options, char *buffer)
{
    int pipefd[2];
    if (pipe(pipefd) < 0)
        return false;

    pid_t pid = fork();

    if (pid == -1) {
        return false;
    }

    bool got_sn = false;

    if (pid == 0) {
        // child
        int devnull = open("/dev/null", O_RDWR);
        if (devnull < 0)
            exit(EXIT_FAILURE);

        close(pipefd[0]);
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        dup2(devnull, STDIN_FILENO);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(devnull);

        char* argv[] = {
            (char*)options->filename,
            NULL
        };
        execvp(argv[0], argv);

        exit(EXIT_FAILURE);
    } else {
        //parent
        close(pipefd[1]);

        int amount_read = 0;
        int i = 0;
        int amount_to_read = MAX_SERIALNUMBER_LEN;
        do {
            amount_read = read(pipefd[0], &buffer[i], amount_to_read);
            if (amount_read > 0) {
                i += amount_read;
                amount_to_read -= amount_read;
            }
        } while(amount_read > 0);

        if (i > 0 && i <= MAX_SERIALNUMBER_LEN) {
            got_sn = true;
            buffer[i] = '\0';
        }

        close(pipefd[0]);
        int wstatus;
        do {
            pid_t w = waitpid(pid, &wstatus, 0);
            if (w != pid) {
                got_sn = false;
                break;
            }

        } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));

        if (WEXITSTATUS(wstatus) != 0) {
            got_sn = false;
        }
    }

    return got_sn;
}
