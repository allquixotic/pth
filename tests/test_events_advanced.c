/*
**  GNU Pth - The GNU Portable Threads
**  Copyright (c) 1999-2006 Ralf S. Engelschall <rse@engelschall.com>
**
**  Test: Advanced event system functionality
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
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

static void test_event_typeof(void)
{
    pth_event_t ev;
    unsigned long type;

    TEST("pth_event_typeof: TIME event");
    ev = pth_event(PTH_EVENT_TIME, pth_timeout(1, 0));
    ASSERT(ev != NULL, "event creation failed");

    type = pth_event_typeof(ev);
    ASSERT(type == PTH_EVENT_TIME, "event type mismatch");

    pth_event_free(ev, PTH_FREE_ALL);
    PASS();
}

static void test_event_typeof_msg(void)
{
    pth_event_t ev;
    unsigned long type;
    pth_msgport_t mp;

    TEST("pth_event_typeof: MSG event");
    mp = pth_msgport_create("test_port");
    ASSERT(mp != NULL, "msgport creation failed");

    ev = pth_event(PTH_EVENT_MSG, mp);
    ASSERT(ev != NULL, "event creation failed");

    type = pth_event_typeof(ev);
    ASSERT(type == PTH_EVENT_MSG, "event type mismatch");

    pth_event_free(ev, PTH_FREE_ALL);
    pth_msgport_destroy(mp);
    PASS();
}

static void test_event_typeof_fd(void)
{
    pth_event_t ev;
    unsigned long type;
    int fds[2];

    TEST("pth_event_typeof: FD event");
    if (pipe(fds) < 0) {
        FAIL("pipe creation failed");
        return;
    }

    ev = pth_event(PTH_EVENT_FD|PTH_UNTIL_FD_READABLE, fds[0]);
    ASSERT(ev != NULL, "event creation failed");

    type = pth_event_typeof(ev);
    ASSERT((type & PTH_EVENT_FD) != 0, "event type should have FD flag");

    pth_event_free(ev, PTH_FREE_ALL);
    close(fds[0]);
    close(fds[1]);
    PASS();
}

static void test_event_concat_two(void)
{
    pth_event_t ev1, ev2, ring;

    TEST("pth_event_concat: two events");
    ev1 = pth_event(PTH_EVENT_TIME, pth_timeout(1, 0));
    ev2 = pth_event(PTH_EVENT_TIME, pth_timeout(2, 0));
    ASSERT(ev1 != NULL && ev2 != NULL, "event creation failed");

    ring = pth_event_concat(ev1, ev2, NULL);
    ASSERT(ring != NULL, "concat failed");

    pth_event_free(ring, PTH_FREE_ALL);
    PASS();
}

static void test_event_concat_three(void)
{
    pth_event_t ev1, ev2, ev3, ring;

    TEST("pth_event_concat: three events");
    ev1 = pth_event(PTH_EVENT_TIME, pth_timeout(1, 0));
    ev2 = pth_event(PTH_EVENT_TIME, pth_timeout(2, 0));
    ev3 = pth_event(PTH_EVENT_TIME, pth_timeout(3, 0));
    ASSERT(ev1 != NULL && ev2 != NULL && ev3 != NULL, "event creation failed");

    ring = pth_event_concat(ev1, ev2, ev3, NULL);
    ASSERT(ring != NULL, "concat failed");

    pth_event_free(ring, PTH_FREE_ALL);
    PASS();
}

static void test_event_isolate(void)
{
    pth_event_t ev1, ev2, ev3, ring, remaining;

    TEST("pth_event_isolate: isolate from ring");
    ev1 = pth_event(PTH_EVENT_TIME, pth_timeout(1, 0));
    ev2 = pth_event(PTH_EVENT_TIME, pth_timeout(2, 0));
    ev3 = pth_event(PTH_EVENT_TIME, pth_timeout(3, 0));
    ASSERT(ev1 != NULL && ev2 != NULL && ev3 != NULL, "event creation failed");

    ring = pth_event_concat(ev1, ev2, ev3, NULL);
    ASSERT(ring != NULL, "concat failed");

    remaining = pth_event_isolate(ev2);
    ASSERT(remaining != NULL, "isolate should return remaining ring");
    ASSERT(remaining == ev1 || remaining == ev3, "remaining should be ev1 or ev3");

    pth_event_free(ev2, PTH_FREE_THIS);
    pth_event_free(remaining, PTH_FREE_ALL);
    PASS();
}

static void test_event_walk_next(void)
{
    pth_event_t ev1, ev2, ev3, ring, walked;

    TEST("pth_event_walk: walk next");
    ev1 = pth_event(PTH_EVENT_TIME, pth_timeout(1, 0));
    ev2 = pth_event(PTH_EVENT_TIME, pth_timeout(2, 0));
    ev3 = pth_event(PTH_EVENT_TIME, pth_timeout(3, 0));
    ASSERT(ev1 != NULL && ev2 != NULL && ev3 != NULL, "event creation failed");

    ring = pth_event_concat(ev1, ev2, ev3, NULL);
    ASSERT(ring != NULL, "concat failed");

    walked = pth_event_walk(ev1, PTH_WALK_NEXT);
    ASSERT(walked != NULL, "walk failed");
    ASSERT(walked != ev1, "walk should return different event");

    pth_event_free(ring, PTH_FREE_ALL);
    PASS();
}

static void test_event_walk_prev(void)
{
    pth_event_t ev1, ev2, ev3, ring, walked;

    TEST("pth_event_walk: walk prev");
    ev1 = pth_event(PTH_EVENT_TIME, pth_timeout(1, 0));
    ev2 = pth_event(PTH_EVENT_TIME, pth_timeout(2, 0));
    ev3 = pth_event(PTH_EVENT_TIME, pth_timeout(3, 0));
    ASSERT(ev1 != NULL && ev2 != NULL && ev3 != NULL, "event creation failed");

    ring = pth_event_concat(ev1, ev2, ev3, NULL);
    ASSERT(ring != NULL, "concat failed");

    walked = pth_event_walk(ev1, PTH_WALK_PREV);
    ASSERT(walked != NULL, "walk failed");
    ASSERT(walked != ev1, "walk should return different event");

    pth_event_free(ring, PTH_FREE_ALL);
    PASS();
}

static void test_event_extract_time(void)
{
    pth_event_t ev;
    pth_time_t timeout;
    int rc;

    TEST("pth_event_extract: TIME event");
    ev = pth_event(PTH_EVENT_TIME, pth_timeout(5, 123456));
    ASSERT(ev != NULL, "event creation failed");

    rc = pth_event_extract(ev, &timeout);
    ASSERT(rc == TRUE, "extract failed");

    pth_event_free(ev, PTH_FREE_ALL);
    PASS();
}

static void test_event_extract_msg(void)
{
    pth_event_t ev;
    pth_msgport_t mp, extracted_mp;
    int rc;

    TEST("pth_event_extract: MSG event");
    mp = pth_msgport_create("extract_test");
    ASSERT(mp != NULL, "msgport creation failed");

    ev = pth_event(PTH_EVENT_MSG, mp);
    ASSERT(ev != NULL, "event creation failed");

    rc = pth_event_extract(ev, &extracted_mp);
    ASSERT(rc == TRUE, "extract failed");
    ASSERT(extracted_mp == mp, "extracted msgport mismatch");

    pth_event_free(ev, PTH_FREE_ALL);
    pth_msgport_destroy(mp);
    PASS();
}

static void test_event_extract_fd(void)
{
    pth_event_t ev;
    int fds[2];
    int extracted_fd;
    int rc;

    TEST("pth_event_extract: FD event");
    if (pipe(fds) < 0) {
        FAIL("pipe creation failed");
        return;
    }

    ev = pth_event(PTH_EVENT_FD|PTH_UNTIL_FD_READABLE, fds[0]);
    ASSERT(ev != NULL, "event creation failed");

    rc = pth_event_extract(ev, &extracted_fd);
    ASSERT(rc == TRUE, "extract failed");
    ASSERT(extracted_fd == fds[0], "extracted fd mismatch");

    pth_event_free(ev, PTH_FREE_ALL);
    close(fds[0]);
    close(fds[1]);
    PASS();
}

static void test_event_fd_readable(void)
{
    pth_event_t ev;
    int fds[2];
    pth_status_t status;

    TEST("pth_event: FD readable");
    if (pipe(fds) < 0) {
        FAIL("pipe creation failed");
        return;
    }

    ev = pth_event(PTH_EVENT_FD|PTH_UNTIL_FD_READABLE, fds[0]);
    ASSERT(ev != NULL, "event creation failed");

    if (write(fds[1], "X", 1) == 1) {
        pth_wait(ev);
        status = pth_event_status(ev);
        ASSERT(status == PTH_STATUS_OCCURRED, "event should have occurred");
    }

    pth_event_free(ev, PTH_FREE_ALL);
    close(fds[0]);
    close(fds[1]);
    PASS();
}

static void test_event_fd_writable(void)
{
    pth_event_t ev;
    int fds[2];
    pth_status_t status;

    TEST("pth_event: FD writable");
    if (pipe(fds) < 0) {
        FAIL("pipe creation failed");
        return;
    }

    ev = pth_event(PTH_EVENT_FD|PTH_UNTIL_FD_WRITEABLE, fds[1]);
    ASSERT(ev != NULL, "event creation failed");

    pth_wait(ev);
    status = pth_event_status(ev);
    ASSERT(status == PTH_STATUS_OCCURRED, "pipe should be writable");

    pth_event_free(ev, PTH_FREE_ALL);
    close(fds[0]);
    close(fds[1]);
    PASS();
}

static void test_event_mutex(void)
{
    pth_event_t ev;
    pth_mutex_t mutex = PTH_MUTEX_INIT;
    pth_status_t status;

    TEST("pth_event: MUTEX event");
    pth_mutex_init(&mutex);

    ev = pth_event(PTH_EVENT_MUTEX, &mutex);
    ASSERT(ev != NULL, "event creation failed");

    pth_wait(ev);
    status = pth_event_status(ev);
    ASSERT(status == PTH_STATUS_OCCURRED, "mutex should be available");

    pth_event_free(ev, PTH_FREE_ALL);
    PASS();
}

static void test_event_cond(void)
{
    pth_event_t ev;
    pth_cond_t cond = PTH_COND_INIT;
    pth_mutex_t mutex = PTH_MUTEX_INIT;

    TEST("pth_event: COND event");
    pth_cond_init(&cond);
    pth_mutex_init(&mutex);

    ev = pth_event(PTH_EVENT_COND, &cond);
    ASSERT(ev != NULL, "event creation failed");

    pth_event_free(ev, PTH_FREE_ALL);
    PASS();
}

static void *tid_test_thread(void *arg __attribute__((unused)))
{
    pth_sleep(1);
    return NULL;
}

static void test_event_tid(void)
{
    pth_event_t ev;
    pth_t tid;
    pth_attr_t attr;

    TEST("pth_event: TID event");
    attr = pth_attr_new();
    pth_attr_set(attr, PTH_ATTR_JOINABLE, FALSE);

    tid = pth_spawn(attr, tid_test_thread, NULL);
    ASSERT(tid != NULL, "thread spawn failed");

    ev = pth_event(PTH_EVENT_TID|PTH_UNTIL_TID_DEAD, tid);
    ASSERT(ev != NULL, "event creation failed");

    pth_wait(ev);

    pth_event_free(ev, PTH_FREE_ALL);
    pth_attr_destroy(attr);
    PASS();
}

static int func_callback_called = 0;

static int func_callback(void *arg __attribute__((unused)))
{
    func_callback_called = 1;
    return TRUE;
}

static void test_event_func(void)
{
    pth_event_t ev;

    TEST("pth_event: FUNC event");
    func_callback_called = 0;

    ev = pth_event(PTH_EVENT_FUNC, func_callback, NULL, pth_time(0, 0));
    ASSERT(ev != NULL, "event creation failed");

    pth_wait(ev);
    ASSERT(func_callback_called == 1, "callback should have been called");

    pth_event_free(ev, PTH_FREE_ALL);
    PASS();
}

static void test_event_select(void)
{
    pth_event_t ev;
    int fds[2];
    fd_set rfds;
    int nfd;
    int rc;

    TEST("pth_event: SELECT event");
    if (pipe(fds) < 0) {
        FAIL("pipe creation failed");
        return;
    }

    FD_ZERO(&rfds);
    FD_SET(fds[0], &rfds);
    nfd = fds[0] + 1;

    rc = -1;
    ev = pth_event(PTH_EVENT_SELECT, &rc, nfd, &rfds, NULL, NULL);
    ASSERT(ev != NULL, "event creation failed");

    if (write(fds[1], "X", 1) < 0) { perror("write"); }
    pth_wait(ev);
    ASSERT(rc > 0, "select should have detected ready fd");

    pth_event_free(ev, PTH_FREE_ALL);
    close(fds[0]);
    close(fds[1]);
    PASS();
}

static void test_event_free_this(void)
{
    pth_event_t ev1, ev2, ring;

    TEST("pth_event_free: PTH_FREE_THIS");
    ev1 = pth_event(PTH_EVENT_TIME, pth_timeout(1, 0));
    ev2 = pth_event(PTH_EVENT_TIME, pth_timeout(2, 0));
    ASSERT(ev1 != NULL && ev2 != NULL, "event creation failed");

    ring = pth_event_concat(ev1, ev2, NULL);
    ASSERT(ring != NULL, "concat failed");

    pth_event_isolate(ev1);
    pth_event_free(ev1, PTH_FREE_THIS);

    pth_event_free(ev2, PTH_FREE_ALL);
    PASS();
}

static void test_event_free_all(void)
{
    pth_event_t ev1, ev2, ev3, ring;

    TEST("pth_event_free: PTH_FREE_ALL");
    ev1 = pth_event(PTH_EVENT_TIME, pth_timeout(1, 0));
    ev2 = pth_event(PTH_EVENT_TIME, pth_timeout(2, 0));
    ev3 = pth_event(PTH_EVENT_TIME, pth_timeout(3, 0));
    ASSERT(ev1 != NULL && ev2 != NULL && ev3 != NULL, "event creation failed");

    ring = pth_event_concat(ev1, ev2, ev3, NULL);
    ASSERT(ring != NULL, "concat failed");

    pth_event_free(ring, PTH_FREE_ALL);
    PASS();
}

static void test_event_sigs(void)
{
    pth_event_t ev;
    sigset_t sigs;

    TEST("pth_event: SIGS event");
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGUSR1);

    ev = pth_event(PTH_EVENT_SIGS, &sigs, NULL);
    ASSERT(ev != NULL, "event creation failed");

    pth_event_free(ev, PTH_FREE_ALL);
    PASS();
}

static void test_event_chain_mode(void)
{
    pth_event_t ev1, ev2;

    TEST("pth_event: CHAIN mode");
    ev1 = pth_event(PTH_EVENT_TIME, pth_timeout(1, 0));
    ASSERT(ev1 != NULL, "event 1 creation failed");

    ev2 = pth_event(PTH_EVENT_TIME|PTH_MODE_CHAIN, ev1, pth_timeout(2, 0));
    ASSERT(ev2 != NULL, "event 2 creation failed");

    pth_event_free(ev1, PTH_FREE_ALL);
    PASS();
}

static void test_event_reuse_mode(void)
{
    pth_event_t ev;
    pth_status_t status;

    TEST("pth_event: REUSE mode");
    ev = pth_event(PTH_EVENT_TIME, pth_timeout(0, 100000));
    ASSERT(ev != NULL, "event creation failed");

    pth_wait(ev);
    status = pth_event_status(ev);
    ASSERT(status == PTH_STATUS_OCCURRED, "event should have occurred");

    ev = pth_event(PTH_EVENT_TIME|PTH_MODE_REUSE, ev, pth_timeout(0, 100000));
    ASSERT(ev != NULL, "event reuse failed");

    pth_wait(ev);
    status = pth_event_status(ev);
    ASSERT(status == PTH_STATUS_OCCURRED, "reused event should have occurred");

    pth_event_free(ev, PTH_FREE_ALL);
    PASS();
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
    printf("========================================\n");
    printf("Advanced Event System Test Suite\n");
    printf("========================================\n\n");

    if (!pth_init()) {
        fprintf(stderr, "ERROR: pth_init() failed\n");
        return 1;
    }

    test_event_typeof();
    test_event_typeof_msg();
    test_event_typeof_fd();
    test_event_concat_two();
    test_event_concat_three();
    test_event_isolate();
    test_event_walk_next();
    test_event_walk_prev();
    test_event_extract_time();
    test_event_extract_msg();
    test_event_extract_fd();
    test_event_fd_readable();
    test_event_fd_writable();
    test_event_mutex();
    test_event_cond();
    test_event_tid();
    test_event_func();
    test_event_select();
    test_event_free_this();
    test_event_free_all();
    test_event_sigs();
    test_event_chain_mode();
    test_event_reuse_mode();

    printf("\n========================================\n");
    printf("Test Results:\n");
    printf("  Total:  %d\n", test_count);
    printf("  Passed: %d\n", test_passed);
    printf("  Failed: %d\n", test_failed);
    printf("========================================\n");

    pth_kill();

    return (test_failed == 0) ? 0 : 1;
}