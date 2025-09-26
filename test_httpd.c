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
**  test_httpd.c: Pth test program (faked HTTP daemon)
*/
                             /* ``Unix is simple. It just takes a
                                  genius to understand its simplicity.''
                                            --- Dennis Ritchie           */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>
#include <unistd.h>

#include "pth.h"

#include "test_common.h"

/*
 * The HTTP request handler
 */

#define MAXREQLINE 1024

static void *handler(void *_arg)
{
    int fd = (int)((long)_arg);
    char caLine[MAXREQLINE];
    char str[1024];
    int n;

    /* read request */
    for (;;) {
        n = pth_readline(fd, caLine, MAXREQLINE);
        if (n < 0) {
            fprintf(stderr, "read error: errno=%d\n", errno);
            close(fd);
            return NULL;
        }
        if (n == 0)
            break;
        if (n == 1 && caLine[0] == '\n')
            break;
        caLine[n-1] = NUL;
    }

    /* simulate a little bit of processing ;) */
    pth_yield(NULL);

    /* generate response */
    sprintf(str, "HTTP/1.0 200 Ok\r\n"
                 "Server: test_httpd/%x\r\n"
                 "Connection: close\r\n"
                 "Content-type: text/plain\r\n"
                 "\r\n"
                 "Just a trivial test for GNU Pth\n"
                 "to show that it's serving data.\r\n", PTH_VERSION);
    pth_write(fd, str, strlen(str));

    /* close connection and let thread die */
    fprintf(stderr, "connection shutdown (fd: %d)\n", fd);
    close(fd);
    return NULL;
}

/*
 * A useless ticker we let run just for fun in parallel
 */

#if 0  /* Currently unused */
static void *ticker(void *_arg)
{
    (void)_arg;
    time_t now;
    char *ct;
    float avload;

    for (;;) {
        pth_sleep(5);
        now = time(NULL);
        ct = ctime(&now);
        ct[strlen(ct)-1] = NUL;
        pth_ctrl(PTH_CTRL_GETAVLOAD, &avload);
        fprintf(stderr, "ticker woken up on %s, average load: %.2f\n",
                ct, avload);
    }
    /* NOTREACHED */
    return NULL;
}
#endif

/* client used for non-interactive mode */
#if 0  /* Currently unused */
static void *self_client(void *parg)
{
    int port = *(int*)parg;
    int fd = -1;
    struct sockaddr_in sa;
    char buf[4096];
    int n;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return NULL;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (connect(fd, (struct sockaddr*)&sa, sizeof(sa)) < 0) return NULL;
    pth_write(fd, "GET / HTTP/1.0\r\n\r\n", 18);
    n = pth_read(fd, buf, sizeof(buf)-1);
    if (n > 0) {
        buf[n] = '\0';
        fprintf(stderr, "client: received %d bytes\n", n);
    }
    close(fd);
    return NULL;
}
#endif

/* client driver used in AUTOTEST mode to generate load */
typedef struct { int port; int secs; } client_args_t;
static void *client_driver(void *parg)
{
    client_args_t *ca = (client_args_t*)parg;
    time_t endt = time(NULL) + ca->secs;
    while (time(NULL) < endt) {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd >= 0) {
            struct sockaddr_in sa; memset(&sa,0,sizeof(sa));
            sa.sin_family = AF_INET; sa.sin_port = htons(ca->port); sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            if (connect(fd,(struct sockaddr*)&sa,sizeof(sa)) == 0) {
                const char *req = "GET / HTTP/1.0\r\n\r\n";
                pth_write(fd, req, strlen(req));
                char buf[1024]; (void)pth_read(fd, buf, sizeof(buf));
            }
            close(fd);
        }
        pth_sleep(1);
    }
    return NULL;
}

/*
 * And the server main procedure
 */

#if defined(FD_SETSIZE)
#define REQ_MAX FD_SETSIZE-100
#else
#define REQ_MAX 100
#endif

static int s;
pth_attr_t attr;

static void myexit(int sig)
{
    (void)sig;
    close(s);
    pth_attr_destroy(attr);
    pth_kill();
    fprintf(stderr, "**Break\n");
    exit(0);
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    struct sockaddr_in peer_addr;
    socklen_t peer_len;
    int sr;
    int port = 8080;

    /* initialize scheduler */
    pth_init();
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT,  myexit);
    signal(SIGTERM, myexit);

    /* create server socket */
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in sar;
    memset(&sar, 0, sizeof(sar));
    sar.sin_family = AF_INET;
    sar.sin_addr.s_addr = INADDR_ANY;
    sar.sin_port = htons(port);
    if (bind(s, (struct sockaddr *)&sar, sizeof(sar)) == -1) {
        perror("bind");
        exit(1);
    }
    if (listen(s, 10) == -1) {
        perror("listen");
        exit(1);
    }

    /* create thread attributes */
    attr = pth_attr_new();
    pth_attr_set(attr, PTH_ATTR_JOINABLE, FALSE);

    /* argument line parsing */
    if (getenv("PTH_AUTOTEST")) {
        int run_secs = 60;
        const char *envs = getenv("PTH_HTTPD_SECS");
        if (envs) { int v = atoi(envs); if (v > 0) run_secs = v; }
        pth_event_t tev = pth_event(PTH_EVENT_TIME, pth_timeout(run_secs,0));
        /* client driver thread repeatedly hits the server */
        client_args_t ca = { .port = port, .secs = run_secs };
        pth_attr_t caa = pth_attr_new(); pth_attr_set(caa, PTH_ATTR_JOINABLE, TRUE); pth_attr_set(caa, PTH_ATTR_NAME, "client-driver");
        pth_spawn(caa, client_driver, &ca); pth_attr_destroy(caa);
        /* accept loop with timeout */
        fd_set rfds;
        while (pth_event_status(tev) != PTH_STATUS_OCCURRED) {
            FD_ZERO(&rfds); FD_SET(s, &rfds);
            int r = pth_select_ev(s+1, &rfds, NULL, NULL, NULL, tev);
            if (r > 0 && FD_ISSET(s, &rfds)) {
                peer_len = sizeof(peer_addr);
                if ((sr = pth_accept(s, (struct sockaddr *)&peer_addr, &peer_len)) != -1) {
                    pth_attr_set(attr, PTH_ATTR_JOINABLE, TRUE);
                    pth_spawn(attr, handler, (void *)((long)sr));
                }
            }
        }
        pth_event_free(tev, PTH_FREE_THIS);
        close(s); pth_attr_destroy(attr); pth_kill(); return 0;
    }

    /* main accept loop */
    for (;;) {
        /* accept next connection */
        peer_len = sizeof(peer_addr);
        if ((sr = pth_accept(s, (struct sockaddr *)&peer_addr, &peer_len)) == -1) {
            perror("accept");
            pth_sleep(1);
            continue;
        }
        if (pth_ctrl(PTH_CTRL_GETTHREADS) >= REQ_MAX) {
            fprintf(stderr, "currently no more connections acceptable\n");
            continue;
        }
        fprintf(stderr, "connection established (fd: %d, ip: %s, port: %d)\n",
                sr, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));

        /* spawn new handling thread for connection */
        pth_spawn(attr, handler, (void *)((long)sr));
    }

}

