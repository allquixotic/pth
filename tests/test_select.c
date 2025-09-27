/*
**  GNU Pth - The GNU Portable Threads
**  Copyright (c) 1999-2006 Ralf S. Engelschall <rse@engelschall.com>
**
**  This file is part of GNU Pth, a non-preemptive thread scheduling
**  library which can be found at http://www.gnu.org/software/pth/.
**
**  This library is free software; you can redistribute it and/or
**  modify it under the terms of the GNU Lesser General Public
**  License as published by the Free Software Foundation; either
**  version 2.1 of the License, or (at your option) any later version.
**
**  This library is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**  Lesser General Public License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License along with this library; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
**  USA, or contact Ralf S. Engelschall <rse@engelschall.com>.
**
**  test_select.c: Pth test program (select)
*/
                             /* ``Most computer problems are located
                                between the keyboard and the chair.'' */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#include "pth.h"

static void *writer(void *arg)
{
    int fd = *(int*)arg;
    const char *msg = "ABC\n";
    pth_sleep(1);
    pth_write(fd, msg, 4);
    close(fd);
    return NULL;
}


/* a useless ticker thread */
static void *ticker(void *_arg)
{
    (void)_arg;
    time_t now;
    fprintf(stderr, "ticker: start\n");
    for (;;) {
        pth_sleep(5);
        now = time(NULL);
        fprintf(stderr, "ticker was woken up on %s", ctime(&now));
    }
    /* NOTREACHED */
    return NULL;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    pth_event_t evt;
    pth_t t_ticker;
    pth_attr_t t_attr;
    fd_set rfds;
    char c;
    int n;

    pth_init();

    fprintf(stderr, "This is TEST_SELECT, a Pth test using select.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Enter data. Hit CTRL-C to stop this test.\n");
    fprintf(stderr, "\n");

    if (getenv("PTH_AUTOTEST")) {
        int fds[2];
        if (pipe(fds) == 0) {
            pth_attr_t wa = pth_attr_new();
            pth_attr_set(wa, PTH_ATTR_NAME, "writer");
            pth_attr_set(wa, PTH_ATTR_JOINABLE, FALSE);
            pth_spawn(wa, writer, &fds[1]);
            pth_attr_destroy(wa);
            dup2(fds[0], STDIN_FILENO);
            close(fds[0]);
        }
    }
    t_attr = pth_attr_new();
    pth_attr_set(t_attr, PTH_ATTR_NAME, "ticker");
    t_ticker = pth_spawn(t_attr, ticker, NULL);
    pth_attr_destroy(t_attr);
    pth_yield(NULL);

    evt = NULL;
    for (;;) {
        if (evt == NULL)
            evt = pth_event(PTH_EVENT_TIME, pth_timeout(10,0));
        else
            evt = pth_event(PTH_EVENT_TIME|PTH_MODE_REUSE, evt, pth_timeout(10,0));
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        n = pth_select_ev(STDIN_FILENO+1, &rfds, NULL, NULL, NULL, evt);
        if (n == -1 && errno == EINTR) {
            fprintf(stderr, "main: timeout - repeating\n");
            continue;
        }
        if (!FD_ISSET(STDIN_FILENO, &rfds)) {
            fprintf(stderr, "main: Hmmmm... strange situation: bit not set\n");
            exit(1);
        }
        fprintf(stderr, "main: select returned %d\n", n);
        while (pth_read(STDIN_FILENO, &c, 1) > 0)
            fprintf(stderr, "main: read stdin '%c'\n", c);
        if (getenv("PTH_AUTOTEST")) break;
    }

    pth_cancel(t_ticker);
    pth_join(t_ticker, NULL);
    pth_event_free(evt, PTH_FREE_THIS);
    pth_kill();
    return 0;
}

