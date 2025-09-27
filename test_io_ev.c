/*
**  GNU Pth - The GNU Portable Threads
**  Copyright (c) 1999-2006 Ralf S. Engelschall <rse@engelschall.com>
**
**  Test: I/O operations with event parameters (_ev variants)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <poll.h>
#include "pth.h"

static int test_count = 0;
static int test_passed = 0;
static int test_failed = 0;

#define TEST(name) do { \
    test_count++; \
    printf("Test %d: %s ... ", test_count, name); \
    fflush(stdout); \
} while (0)

#define PASS() do { \
    test_passed++; \
    printf("OK\n"); \
} while (0)

#define FAIL(msg) do { \
    test_failed++; \
    printf("FAILED: %s\n", msg); \
} while (0)

#define ASSERT(cond, msg) do { \
    if (!(cond)) { \
        FAIL(msg); \
        return; \
    } \
} while (0)

static void test_read_ev_basic(void)
{
    int fds[2];
    char buf[64];
    ssize_t n;
    pth_event_t ev;

    TEST("pth_read_ev: basic read with event");
    if (pipe(fds) < 0) {
        FAIL("pipe creation failed");
        return;
    }

    ev = pth_event(PTH_EVENT_TIME, pth_timeout(2, 0));
    ASSERT(ev != NULL, "event creation failed");

    if (write(fds[1], "Hello", 5) < 0) { perror("write"); }
    n = pth_read_ev(fds[0], buf, sizeof(buf), ev);
    ASSERT(n == 5, "read size mismatch");
    ASSERT(memcmp(buf, "Hello", 5) == 0, "read data mismatch");

    pth_event_free(ev, PTH_FREE_ALL);
    close(fds[0]);
    close(fds[1]);
    PASS();
}

static void test_read_ev_timeout(void)
{
    int fds[2];
    char buf[64];
    ssize_t n;
    pth_event_t ev;

    TEST("pth_read_ev: timeout event");
    if (pipe(fds) < 0) {
        FAIL("pipe creation failed");
        return;
    }

    ev = pth_event(PTH_EVENT_TIME, pth_timeout(0, 100000));
    ASSERT(ev != NULL, "event creation failed");

    n = pth_read_ev(fds[0], buf, sizeof(buf), ev);
    ASSERT(n <= 0, "should timeout or return error");

    pth_event_free(ev, PTH_FREE_ALL);
    close(fds[0]);
    close(fds[1]);
    PASS();
}

static void test_write_ev_basic(void)
{
    int fds[2];
    ssize_t n;
    pth_event_t ev;

    TEST("pth_write_ev: basic write with event");
    if (pipe(fds) < 0) {
        FAIL("pipe creation failed");
        return;
    }

    ev = pth_event(PTH_EVENT_TIME, pth_timeout(2, 0));
    ASSERT(ev != NULL, "event creation failed");

    n = pth_write_ev(fds[1], "Test", 4, ev);
    ASSERT(n == 4, "write size mismatch");

    pth_event_free(ev, PTH_FREE_ALL);
    close(fds[0]);
    close(fds[1]);
    PASS();
}

static void test_readv_ev_basic(void)
{
    int fds[2];
    struct iovec iov[2];
    char buf1[32], buf2[32];
    ssize_t n;
    pth_event_t ev;

    TEST("pth_readv_ev: vectored read with event");
    if (pipe(fds) < 0) {
        FAIL("pipe creation failed");
        return;
    }

    ev = pth_event(PTH_EVENT_TIME, pth_timeout(2, 0));
    ASSERT(ev != NULL, "event creation failed");

    if (write(fds[1], "HelloWorld", 10) < 0) { perror("write"); }

    iov[0].iov_base = buf1;
    iov[0].iov_len = 5;
    iov[1].iov_base = buf2;
    iov[1].iov_len = 5;

    n = pth_readv_ev(fds[0], iov, 2, ev);
    ASSERT(n == 10, "readv size mismatch");
    ASSERT(memcmp(buf1, "Hello", 5) == 0, "buf1 mismatch");
    ASSERT(memcmp(buf2, "World", 5) == 0, "buf2 mismatch");

    pth_event_free(ev, PTH_FREE_ALL);
    close(fds[0]);
    close(fds[1]);
    PASS();
}

static void test_writev_ev_basic(void)
{
    int fds[2];
    struct iovec iov[2];
    char buf1[] = "Hello";
    char buf2[] = "World";
    ssize_t n;
    pth_event_t ev;

    TEST("pth_writev_ev: vectored write with event");
    if (pipe(fds) < 0) {
        FAIL("pipe creation failed");
        return;
    }

    ev = pth_event(PTH_EVENT_TIME, pth_timeout(2, 0));
    ASSERT(ev != NULL, "event creation failed");

    iov[0].iov_base = buf1;
    iov[0].iov_len = 5;
    iov[1].iov_base = buf2;
    iov[1].iov_len = 5;

    n = pth_writev_ev(fds[1], iov, 2, ev);
    ASSERT(n == 10, "writev size mismatch");

    pth_event_free(ev, PTH_FREE_ALL);
    close(fds[0]);
    close(fds[1]);
    PASS();
}

static void test_select_ev_basic(void)
{
    int fds[2];
    fd_set rfds;
    struct timeval tv;
    int rc;
    pth_event_t ev;

    TEST("pth_select_ev: select with event");
    if (pipe(fds) < 0) {
        FAIL("pipe creation failed");
        return;
    }

    ev = pth_event(PTH_EVENT_TIME, pth_timeout(2, 0));
    ASSERT(ev != NULL, "event creation failed");

    if (write(fds[1], "X", 1) < 0) { perror("write"); }

    FD_ZERO(&rfds);
    FD_SET(fds[0], &rfds);
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    rc = pth_select_ev(fds[0] + 1, &rfds, NULL, NULL, &tv, ev);
    ASSERT(rc > 0, "select should detect readable fd");
    ASSERT(FD_ISSET(fds[0], &rfds), "fd should be set");

    pth_event_free(ev, PTH_FREE_ALL);
    close(fds[0]);
    close(fds[1]);
    PASS();
}

static void test_poll_ev_basic(void)
{
    int fds[2];
    struct pollfd pfd;
    int rc;
    pth_event_t ev;

    TEST("pth_poll_ev: poll with event");
    if (pipe(fds) < 0) {
        FAIL("pipe creation failed");
        return;
    }

    ev = pth_event(PTH_EVENT_TIME, pth_timeout(2, 0));
    ASSERT(ev != NULL, "event creation failed");

    if (write(fds[1], "X", 1) < 0) { perror("write"); }

    pfd.fd = fds[0];
    pfd.events = POLLIN;
    pfd.revents = 0;

    rc = pth_poll_ev(&pfd, 1, 1000, ev);
    ASSERT(rc > 0, "poll should detect readable fd");
    ASSERT((pfd.revents & POLLIN) != 0, "POLLIN should be set");

    pth_event_free(ev, PTH_FREE_ALL);
    close(fds[0]);
    close(fds[1]);
    PASS();
}

static void test_accept_ev_basic(void)
{
    int listen_fd;
    int conn_fd __attribute__((unused));
    struct sockaddr_in addr;
    socklen_t addrlen;
    pth_event_t ev;

    TEST("pth_accept_ev: accept with event (no connection)");

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT(listen_fd >= 0, "socket creation failed");

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(listen_fd);
        FAIL("bind failed");
        return;
    }

    if (listen(listen_fd, 5) < 0) {
        close(listen_fd);
        FAIL("listen failed");
        return;
    }

    ev = pth_event(PTH_EVENT_TIME, pth_timeout(0, 100000));
    ASSERT(ev != NULL, "event creation failed");

    addrlen = sizeof(addr);
    conn_fd = pth_accept_ev(listen_fd, (struct sockaddr *)&addr, &addrlen, ev);

    pth_event_free(ev, PTH_FREE_ALL);
    close(listen_fd);

    PASS();
}

static void test_connect_ev_basic(void)
{
    int listen_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t addrlen;
    pth_event_t ev;
    int rc;

    TEST("pth_connect_ev: connect with event");

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT(listen_fd >= 0, "socket creation failed");

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(listen_fd);
        FAIL("bind failed");
        return;
    }

    addrlen = sizeof(addr);
    if (getsockname(listen_fd, (struct sockaddr *)&addr, &addrlen) < 0) {
        close(listen_fd);
        FAIL("getsockname failed");
        return;
    }

    if (listen(listen_fd, 5) < 0) {
        close(listen_fd);
        FAIL("listen failed");
        return;
    }

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT(client_fd >= 0, "client socket creation failed");

    ev = pth_event(PTH_EVENT_TIME, pth_timeout(2, 0));
    ASSERT(ev != NULL, "event creation failed");

    rc = pth_connect_ev(client_fd, (struct sockaddr *)&addr, addrlen, ev);

    pth_event_free(ev, PTH_FREE_ALL);
    close(client_fd);
    close(listen_fd);

    if (rc == 0) {
        PASS();
    } else {
        FAIL("connect failed");
    }
}

static void test_recv_ev_basic(void)
{
    int fds[2];
    char buf[64];
    ssize_t n;
    pth_event_t ev;

    TEST("pth_recv_ev: recv with event");
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) < 0) {
        FAIL("socketpair creation failed");
        return;
    }

    ev = pth_event(PTH_EVENT_TIME, pth_timeout(2, 0));
    ASSERT(ev != NULL, "event creation failed");

    if (write(fds[1], "Test", 4) < 0) { perror("write"); }
    n = pth_recv_ev(fds[0], buf, sizeof(buf), 0, ev);
    ASSERT(n == 4, "recv size mismatch");
    ASSERT(memcmp(buf, "Test", 4) == 0, "recv data mismatch");

    pth_event_free(ev, PTH_FREE_ALL);
    close(fds[0]);
    close(fds[1]);
    PASS();
}

static void test_send_ev_basic(void)
{
    int fds[2];
    ssize_t n;
    pth_event_t ev;

    TEST("pth_send_ev: send with event");
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) < 0) {
        FAIL("socketpair creation failed");
        return;
    }

    ev = pth_event(PTH_EVENT_TIME, pth_timeout(2, 0));
    ASSERT(ev != NULL, "event creation failed");

    n = pth_send_ev(fds[1], "Test", 4, 0, ev);
    ASSERT(n == 4, "send size mismatch");

    pth_event_free(ev, PTH_FREE_ALL);
    close(fds[0]);
    close(fds[1]);
    PASS();
}

static void test_recvfrom_ev_basic(void)
{
    int fds[2];
    char buf[64];
    struct sockaddr_storage addr;
    socklen_t addrlen;
    ssize_t n;
    pth_event_t ev;

    TEST("pth_recvfrom_ev: recvfrom with event");
    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, fds) < 0) {
        FAIL("socketpair creation failed");
        return;
    }

    ev = pth_event(PTH_EVENT_TIME, pth_timeout(2, 0));
    ASSERT(ev != NULL, "event creation failed");

    if (write(fds[1], "Data", 4) < 0) { perror("write"); }

    addrlen = sizeof(addr);
    n = pth_recvfrom_ev(fds[0], buf, sizeof(buf), 0,
                        (struct sockaddr *)&addr, &addrlen, ev);
    ASSERT(n == 4, "recvfrom size mismatch");
    ASSERT(memcmp(buf, "Data", 4) == 0, "recvfrom data mismatch");

    pth_event_free(ev, PTH_FREE_ALL);
    close(fds[0]);
    close(fds[1]);
    PASS();
}

static void test_sendto_ev_basic(void)
{
    int fds[2];
    ssize_t n;
    pth_event_t ev;

    TEST("pth_sendto_ev: sendto with event");
    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, fds) < 0) {
        FAIL("socketpair creation failed");
        return;
    }

    ev = pth_event(PTH_EVENT_TIME, pth_timeout(2, 0));
    ASSERT(ev != NULL, "event creation failed");

    n = pth_sendto_ev(fds[1], "Data", 4, 0, NULL, 0, ev);
    ASSERT(n == 4, "sendto size mismatch");

    pth_event_free(ev, PTH_FREE_ALL);
    close(fds[0]);
    close(fds[1]);
    PASS();
}

static void test_sigwait_ev_basic(void)
{
    sigset_t sigs;
    int sig;
    pth_event_t ev;
    int rc __attribute__((unused));

    TEST("pth_sigwait_ev: sigwait with timeout event");
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGUSR1);

    ev = pth_event(PTH_EVENT_TIME, pth_timeout(0, 100000));
    ASSERT(ev != NULL, "event creation failed");

    rc = pth_sigwait_ev(&sigs, &sig, ev);

    pth_event_free(ev, PTH_FREE_ALL);
    PASS();
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
    printf("========================================\n");
    printf("I/O Event Variants Test Suite\n");
    printf("========================================\n\n");

    if (!pth_init()) {
        fprintf(stderr, "ERROR: pth_init() failed\n");
        return 1;
    }

    test_read_ev_basic();
    test_read_ev_timeout();
    test_write_ev_basic();
    test_readv_ev_basic();
    test_writev_ev_basic();
    test_select_ev_basic();
    test_poll_ev_basic();
    test_accept_ev_basic();
    test_connect_ev_basic();
    test_recv_ev_basic();
    test_send_ev_basic();
    test_recvfrom_ev_basic();
    test_sendto_ev_basic();
    test_sigwait_ev_basic();

    printf("\n========================================\n");
    printf("Test Results:\n");
    printf("  Total:  %d\n", test_count);
    printf("  Passed: %d\n", test_passed);
    printf("  Failed: %d\n", test_failed);
    printf("========================================\n");

    pth_kill();

    return (test_failed == 0) ? 0 : 1;
}