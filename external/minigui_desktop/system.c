/*
 * Rockchip App
 *
 * Copyright (C) 2017 Rockchip Electronics Co., Ltd.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL), available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "system.h"

#include <errno.h>
#include <paths.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

# define TEMP_FAILURE_RETRY(expression) \
  (__extension__                          \
    ({ long int __result;                       \
       do __result = (long int) (expression);                  \
       while (__result == -1L && errno == EINTR);              \
       __result; }))

int system_fd_closexec(const char *command)
{
    int status = 0;
    pid_t pid;

    if (command == NULL)
        return 1;

    if ((pid = vfork()) < 0)
        return -1;

    if (pid == 0)
    {
        int i = 0;
        int stdin_fd = fileno(stdin);
        int stdout_fd = fileno(stdout);
        int stderr_fd = fileno(stderr);
        long sc_open_max = sysconf(_SC_OPEN_MAX);
        if (sc_open_max < 0)
        {
            fprintf(stderr, "Warning, sc_open_max is unlimited!\n");
            sc_open_max = 20000; /* enough? */
        }
        /* close all descriptors in child sysconf(_SC_OPEN_MAX) */
        for (; i < sc_open_max; i++)
        {
            if (i == stdin_fd || i == stdout_fd || i == stderr_fd)
                continue;
            close(i);
        }

        execl(_PATH_BSHELL, "sh", "-c", command, (char *)0);
        _exit(127);
    }

    if (TEMP_FAILURE_RETRY(waitpid(pid, &status, 0)) != pid)
        status = -1;

    return status;
}

int runapp_result(char *cmd)
{
    char buffer[BUFSIZ] = {0};
    FILE *read_fp;
    int chars_read;
    int ret;

    read_fp = popen(cmd, "r");
    if (read_fp != NULL)
    {
        chars_read = fread(buffer, sizeof(char), BUFSIZ - 1, read_fp);
        if (chars_read > 0)
            ret = 1;
        else
            ret = -1;
        pclose(read_fp);
    }
    else
    {
        ret = -1;
    }

    return ret;
}
