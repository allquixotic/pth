/*
**  GNU Pth - The GNU Portable Threads
**  Copyright (c) 1999-2006 Ralf S. Engelschall <rse@engelschall.com>
**
**  This file is part of GNU Pth, a non-preemptive thread scheduling
**  library which can be found at http://www.gnu.org/software/pth/.
**
**  test_io.c: I/O API coverage test
**  Tests I/O functions not covered by existing tests
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>

#include "pth.h"

#define TEST_FAILED(msg) \
    do { \
        fprintf(stderr, "FAILED: %s (errno=%d)\n", msg, errno); \
        pth_kill(); \
        exit(1); \
    } while (0)

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) TEST_FAILED(msg); \
    } while (0)

static void test_pth_nanosleep(void)
{
    struct timespec req, rem;
    int rc;

    fprintf(stderr, "\nTesting pth_nanosleep...\n");

    req.tv_sec = 0;
    req.tv_nsec = 50000000;

    rc = pth_nanosleep(&req, &rem);
    TEST_ASSERT(rc == 0, "pth_nanosleep failed");

    fprintf(stderr, "  PASSED: pth_nanosleep works correctly\n");
}

static void *writer_thread(void *arg)
{
    int fd = *(int *)arg;
    const char *msg = "Hello from writer thread\n";

    pth_sleep(1);
    pth_write(fd, msg, strlen(msg));
    close(fd);

    return NULL;
}

static void test_pth_read_write(void)
{
    int fds[2];
    char buf[128];
    ssize_t n;
    pth_attr_t attr;
    pth_t tid;

    fprintf(stderr, "\nTesting pth_read and pth_write...\n");

    if (pipe(fds) != 0)
        TEST_FAILED("pipe creation failed");

    attr = pth_attr_new();
    pth_attr_set(attr, PTH_ATTR_NAME, "writer");
    pth_attr_set(attr, PTH_ATTR_JOINABLE, FALSE);
    tid = pth_spawn(attr, writer_thread, &fds[1]);
    TEST_ASSERT(tid != NULL, "pth_spawn failed");
    pth_attr_destroy(attr);

    memset(buf, 0, sizeof(buf));
    n = pth_read(fds[0], buf, sizeof(buf) - 1);
    TEST_ASSERT(n > 0, "pth_read failed");
    TEST_ASSERT(strcmp(buf, "Hello from writer thread\n") == 0, "data mismatch");

    fprintf(stderr, "  read %zd bytes: %s", n, buf);

    close(fds[0]);

    fprintf(stderr, "  PASSED: pth_read and pth_write work correctly\n");
}

static void test_pth_readv_writev(void)
{
    int fds[2];
    struct iovec iov_write[3], iov_read[3];
    char buf1[] = "Hello ";
    char buf2[] = "from ";
    char buf3[] = "writev!\n";
    char read_buf1[10], read_buf2[10], read_buf3[10];
    ssize_t n;

    fprintf(stderr, "\nTesting pth_readv and pth_writev...\n");

    if (pipe(fds) != 0)
        TEST_FAILED("pipe creation failed");

    iov_write[0].iov_base = buf1;
    iov_write[0].iov_len = strlen(buf1);
    iov_write[1].iov_base = buf2;
    iov_write[1].iov_len = strlen(buf2);
    iov_write[2].iov_base = buf3;
    iov_write[2].iov_len = strlen(buf3);

    n = pth_writev(fds[1], iov_write, 3);
    TEST_ASSERT(n == (ssize_t)(strlen(buf1) + strlen(buf2) + strlen(buf3)),
                "pth_writev wrote wrong amount");
    fprintf(stderr, "  wrote %zd bytes with writev\n", n);

    memset(read_buf1, 0, sizeof(read_buf1));
    memset(read_buf2, 0, sizeof(read_buf2));
    memset(read_buf3, 0, sizeof(read_buf3));

    iov_read[0].iov_base = read_buf1;
    iov_read[0].iov_len = 6;
    iov_read[1].iov_base = read_buf2;
    iov_read[1].iov_len = 5;
    iov_read[2].iov_base = read_buf3;
    iov_read[2].iov_len = 8;

    n = pth_readv(fds[0], iov_read, 3);
    TEST_ASSERT(n == (ssize_t)(strlen(buf1) + strlen(buf2) + strlen(buf3)),
                "pth_readv read wrong amount");
    fprintf(stderr, "  read %zd bytes with readv: '%s%s%s'\n",
            n, read_buf1, read_buf2, read_buf3);

    TEST_ASSERT(strcmp(read_buf1, "Hello ") == 0, "readv buf1 mismatch");
    TEST_ASSERT(strcmp(read_buf2, "from ") == 0, "readv buf2 mismatch");
    TEST_ASSERT(strcmp(read_buf3, "writev!\n") == 0, "readv buf3 mismatch");

    close(fds[0]);
    close(fds[1]);

    fprintf(stderr, "  PASSED: pth_readv and pth_writev work correctly\n");
}

static void test_pth_pread_pwrite(void)
{
    char filename[] = "/tmp/pth_test_XXXXXX";
    int fd;
    char write_buf[] = "0123456789ABCDEFGHIJ";
    char read_buf[32];
    ssize_t n;

    fprintf(stderr, "\nTesting pth_pread and pth_pwrite...\n");

    fd = mkstemp(filename);
    TEST_ASSERT(fd >= 0, "mkstemp failed");

    n = pth_pwrite(fd, write_buf, 20, 0);
    TEST_ASSERT(n == 20, "pth_pwrite failed");
    fprintf(stderr, "  wrote %zd bytes at offset 0\n", n);

    memset(read_buf, 0, sizeof(read_buf));
    n = pth_pread(fd, read_buf, 10, 5);
    TEST_ASSERT(n == 10, "pth_pread failed");
    fprintf(stderr, "  read %zd bytes at offset 5: '%s'\n", n, read_buf);
    TEST_ASSERT(strncmp(read_buf, "56789ABCDE", 10) == 0, "pread data mismatch");

    memset(read_buf, 0, sizeof(read_buf));
    n = pth_pread(fd, read_buf, 5, 0);
    TEST_ASSERT(n == 5, "pth_pread at offset 0 failed");
    fprintf(stderr, "  read %zd bytes at offset 0: '%s'\n", n, read_buf);
    TEST_ASSERT(strncmp(read_buf, "01234", 5) == 0, "pread offset 0 data mismatch");

    close(fd);
    unlink(filename);

    fprintf(stderr, "  PASSED: pth_pread and pth_pwrite work correctly\n");
}

static void test_pth_poll(void)
{
    int fds[2];
    struct pollfd pfd[1];
    int rc;
    char byte = 'X';

    fprintf(stderr, "\nTesting pth_poll...\n");

    if (pipe(fds) != 0)
        TEST_FAILED("pipe creation failed");

    pfd[0].fd = fds[0];
    pfd[0].events = POLLIN;
    pfd[0].revents = 0;

    rc = pth_poll(pfd, 1, 100);
    TEST_ASSERT(rc == 0, "pth_poll should timeout with no data");
    fprintf(stderr, "  poll timed out as expected (no data)\n");

    if (write(fds[1], &byte, 1) != 1)
        TEST_FAILED("write failed");

    rc = pth_poll(pfd, 1, 1000);
    TEST_ASSERT(rc == 1, "pth_poll should return 1 when data available");
    TEST_ASSERT(pfd[0].revents & POLLIN, "POLLIN should be set");
    fprintf(stderr, "  poll returned %d, revents=0x%x (data available)\n",
            rc, pfd[0].revents);

    if (read(fds[0], &byte, 1) != 1)
        TEST_FAILED("read failed");

    close(fds[0]);
    close(fds[1]);

    fprintf(stderr, "  PASSED: pth_poll works correctly\n");
}

static void test_pth_select(void)
{
    int fds[2];
    fd_set readfds;
    struct timeval tv;
    int rc;
    char byte = 'Y';

    fprintf(stderr, "\nTesting pth_select...\n");

    if (pipe(fds) != 0)
        TEST_FAILED("pipe creation failed");

    FD_ZERO(&readfds);
    FD_SET(fds[0], &readfds);
    tv.tv_sec = 0;
    tv.tv_usec = 100000;

    rc = pth_select(fds[0] + 1, &readfds, NULL, NULL, &tv);
    TEST_ASSERT(rc == 0, "pth_select should timeout with no data");
    fprintf(stderr, "  select timed out as expected (no data)\n");

    if (write(fds[1], &byte, 1) != 1)
        TEST_FAILED("write failed");

    FD_ZERO(&readfds);
    FD_SET(fds[0], &readfds);
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    rc = pth_select(fds[0] + 1, &readfds, NULL, NULL, &tv);
    TEST_ASSERT(rc == 1, "pth_select should return 1 when data available");
    TEST_ASSERT(FD_ISSET(fds[0], &readfds), "fd should be set in readfds");
    fprintf(stderr, "  select returned %d (data available)\n", rc);

    if (read(fds[0], &byte, 1) != 1)
        TEST_FAILED("read failed");

    close(fds[0]);
    close(fds[1]);

    fprintf(stderr, "  PASSED: pth_select works correctly\n");
}

static void *tcp_server_thread(void *arg)
{
    int *listen_fd_ptr = (int *)arg;
    int listen_fd, client_fd;
    struct sockaddr_in addr, client_addr;
    socklen_t client_len;
    char buf[128];
    ssize_t n;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
        return NULL;

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(listen_fd);
        return NULL;
    }

    socklen_t addr_len = sizeof(addr);
    if (getsockname(listen_fd, (struct sockaddr *)&addr, &addr_len) < 0) {
        close(listen_fd);
        return NULL;
    }

    *listen_fd_ptr = ntohs(addr.sin_port);

    if (listen(listen_fd, 1) < 0) {
        close(listen_fd);
        return NULL;
    }

    fprintf(stderr, "  server: listening on port %d\n", *listen_fd_ptr);

    client_len = sizeof(client_addr);
    client_fd = pth_accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        close(listen_fd);
        return NULL;
    }

    fprintf(stderr, "  server: accepted connection\n");

    n = pth_read(client_fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        fprintf(stderr, "  server: received: %s", buf);

        const char *response = "Hello from server\n";
        pth_write(client_fd, response, strlen(response));
    }

    close(client_fd);
    close(listen_fd);

    return NULL;
}

static void test_pth_accept_connect(void)
{
    pth_t server_tid;
    int port = 0;
    int client_fd;
    struct sockaddr_in addr;
    char send_buf[] = "Hello from client\n";
    char recv_buf[128];
    ssize_t n;

    fprintf(stderr, "\nTesting pth_accept and pth_connect...\n");

    server_tid = pth_spawn(PTH_ATTR_DEFAULT, tcp_server_thread, &port);
    TEST_ASSERT(server_tid != NULL, "pth_spawn server failed");

    pth_sleep(1);

    TEST_ASSERT(port > 0, "server did not bind to a port");

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    TEST_ASSERT(client_fd >= 0, "socket creation failed");

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(port);

    fprintf(stderr, "  client: connecting to port %d\n", port);

    int rc = pth_connect(client_fd, (struct sockaddr *)&addr, sizeof(addr));
    TEST_ASSERT(rc == 0, "pth_connect failed");

    fprintf(stderr, "  client: connected\n");

    n = pth_write(client_fd, send_buf, strlen(send_buf));
    TEST_ASSERT(n == (ssize_t)strlen(send_buf), "pth_write failed");

    memset(recv_buf, 0, sizeof(recv_buf));
    n = pth_read(client_fd, recv_buf, sizeof(recv_buf) - 1);
    TEST_ASSERT(n > 0, "pth_read failed");
    fprintf(stderr, "  client: received: %s", recv_buf);

    TEST_ASSERT(strcmp(recv_buf, "Hello from server\n") == 0, "data mismatch");

    close(client_fd);
    pth_join(server_tid, NULL);

    fprintf(stderr, "  PASSED: pth_accept and pth_connect work correctly\n");
}

static void test_pth_recv_send(void)
{
    int fds[2];
    char send_buf[] = "Test message";
    char recv_buf[128];
    ssize_t n;

    fprintf(stderr, "\nTesting pth_recv and pth_send (with socketpair)...\n");

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0)
        TEST_FAILED("socketpair failed");

    n = pth_send(fds[0], send_buf, strlen(send_buf), 0);
    TEST_ASSERT(n == (ssize_t)strlen(send_buf), "pth_send failed");
    fprintf(stderr, "  sent %zd bytes\n", n);

    memset(recv_buf, 0, sizeof(recv_buf));
    n = pth_recv(fds[1], recv_buf, sizeof(recv_buf) - 1, 0);
    TEST_ASSERT(n == (ssize_t)strlen(send_buf), "pth_recv failed");
    TEST_ASSERT(strcmp(recv_buf, send_buf) == 0, "data mismatch");
    fprintf(stderr, "  received %zd bytes: '%s'\n", n, recv_buf);

    close(fds[0]);
    close(fds[1]);

    fprintf(stderr, "  PASSED: pth_recv and pth_send work correctly\n");
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    int rc;

    fprintf(stderr, "=== TEST_IO: I/O API Coverage Test ===\n");

    rc = pth_init();
    TEST_ASSERT(rc == TRUE, "pth_init failed");

    test_pth_nanosleep();
    test_pth_read_write();
    test_pth_readv_writev();
    test_pth_pread_pwrite();
    test_pth_poll();
    test_pth_select();
    test_pth_accept_connect();
    test_pth_recv_send();

    pth_kill();

    fprintf(stderr, "\n=== ALL I/O TESTS PASSED ===\n");
    return 0;
}