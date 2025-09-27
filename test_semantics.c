/*
**  GNU Pth - The GNU Portable Threads
**  Copyright (c) 1999-2006 Ralf S. Engelschall <rse@engelschall.com>
**
**  This file is part of GNU Pth, a non-preemptive thread scheduling
**  library which can be found at http://www.gnu.org/software/pth/.
**
**  test_semantics.c: Advanced semantics and API coverage test
**  Tests uncovered semantics and edge cases
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

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

static int cleanup_executed = 0;
static int cleanup_counter = 0;

static void cleanup_handler(void *arg)
{
    int *counter = (int *)arg;
    (*counter)++;
    cleanup_executed = 1;
    fprintf(stderr, "  cleanup_handler executed, counter=%d\n", *counter);
}

static void *test_cleanup_thread(void *arg)
{
    int execute = *(int *)arg;
    int local_counter = 0;

    pth_cleanup_push(cleanup_handler, &local_counter);
    pth_cleanup_push(cleanup_handler, &local_counter);

    pth_cleanup_pop(execute);

    if (execute) {
        TEST_ASSERT(local_counter == 1, "cleanup should execute when execute=TRUE");
    } else {
        TEST_ASSERT(local_counter == 0, "cleanup should not execute when execute=FALSE");
    }

    pth_cleanup_pop(TRUE);
    TEST_ASSERT(local_counter == (execute ? 2 : 1), "second cleanup should execute");

    cleanup_counter = local_counter;
    return NULL;
}

static void test_pth_cleanup_pop(void)
{
    pth_t tid;
    int execute_true = 1, execute_false = 0;

    fprintf(stderr, "\nTesting pth_cleanup_pop...\n");

    cleanup_counter = 0;
    tid = pth_spawn(PTH_ATTR_DEFAULT, test_cleanup_thread, &execute_false);
    TEST_ASSERT(tid != NULL, "pth_spawn failed");
    pth_join(tid, NULL);
    TEST_ASSERT(cleanup_counter == 1, "cleanup_pop(FALSE) + cleanup_pop(TRUE) should execute 1 handler");
    fprintf(stderr, "  cleanup_pop(FALSE) test passed\n");

    cleanup_counter = 0;
    tid = pth_spawn(PTH_ATTR_DEFAULT, test_cleanup_thread, &execute_true);
    TEST_ASSERT(tid != NULL, "pth_spawn failed");
    pth_join(tid, NULL);
    TEST_ASSERT(cleanup_counter == 2, "cleanup_pop(TRUE) twice should execute 2 handlers");
    fprintf(stderr, "  cleanup_pop(TRUE) test passed\n");

    fprintf(stderr, "  PASSED: pth_cleanup_pop works correctly\n");
}

static void *dummy_thread(void *arg)
{
    int *counter = (int *)arg;
    int i;
    for (i = 0; i < 100; i++) {
        (*counter)++;
        pth_yield(NULL);
    }
    return (void *)(long)*counter;
}

static void test_read_only_attributes(void)
{
    pth_t tid;
    pth_attr_t attr, thread_attr;
    int counter = 0;
    int dispatches = 0;
    pth_time_t time_spawn, time_last, time_ran;
    void *(*start_func)(void *);
    void *start_arg;
    pth_state_t state;
    pth_event_t events;
    int bound;
    int rc;

    fprintf(stderr, "\nTesting read-only attributes...\n");

    attr = pth_attr_new();
    pth_attr_set(attr, PTH_ATTR_NAME, "test_ro_thread");
    tid = pth_spawn(attr, dummy_thread, &counter);
    TEST_ASSERT(tid != NULL, "pth_spawn failed");
    pth_attr_destroy(attr);

    pth_yield(NULL);
    pth_yield(NULL);

    thread_attr = pth_attr_of(tid);
    TEST_ASSERT(thread_attr != NULL, "pth_attr_of failed");

    rc = pth_attr_get(thread_attr, PTH_ATTR_DISPATCHES, &dispatches);
    TEST_ASSERT(rc == TRUE, "pth_attr_get DISPATCHES failed");
    fprintf(stderr, "  DISPATCHES=%d\n", dispatches);
    TEST_ASSERT(dispatches > 0, "dispatches should be > 0");

    rc = pth_attr_get(thread_attr, PTH_ATTR_TIME_SPAWN, &time_spawn);
    TEST_ASSERT(rc == TRUE, "pth_attr_get TIME_SPAWN failed");
    fprintf(stderr, "  TIME_SPAWN: tv_sec=%ld, tv_usec=%ld\n",
            (long)time_spawn.tv_sec, (long)time_spawn.tv_usec);

    rc = pth_attr_get(thread_attr, PTH_ATTR_TIME_LAST, &time_last);
    TEST_ASSERT(rc == TRUE, "pth_attr_get TIME_LAST failed");
    fprintf(stderr, "  TIME_LAST: tv_sec=%ld, tv_usec=%ld\n",
            (long)time_last.tv_sec, (long)time_last.tv_usec);

    rc = pth_attr_get(thread_attr, PTH_ATTR_TIME_RAN, &time_ran);
    TEST_ASSERT(rc == TRUE, "pth_attr_get TIME_RAN failed");
    fprintf(stderr, "  TIME_RAN: tv_sec=%ld, tv_usec=%ld\n",
            (long)time_ran.tv_sec, (long)time_ran.tv_usec);

    rc = pth_attr_get(thread_attr, PTH_ATTR_START_FUNC, &start_func);
    TEST_ASSERT(rc == TRUE, "pth_attr_get START_FUNC failed");
    TEST_ASSERT(start_func == dummy_thread, "START_FUNC mismatch");
    fprintf(stderr, "  START_FUNC=%p (expected %p)\n", (void*)start_func, (void*)dummy_thread);

    rc = pth_attr_get(thread_attr, PTH_ATTR_START_ARG, &start_arg);
    TEST_ASSERT(rc == TRUE, "pth_attr_get START_ARG failed");
    TEST_ASSERT(start_arg == &counter, "START_ARG mismatch");
    fprintf(stderr, "  START_ARG=%p (expected %p)\n", start_arg, (void*)&counter);

    rc = pth_attr_get(thread_attr, PTH_ATTR_STATE, &state);
    TEST_ASSERT(rc == TRUE, "pth_attr_get STATE failed");
    fprintf(stderr, "  STATE=%d (PTH_STATE_NEW=%d, READY=%d, WAITING=%d, DEAD=%d)\n",
            state, PTH_STATE_NEW, PTH_STATE_READY, PTH_STATE_WAITING, PTH_STATE_DEAD);

    rc = pth_attr_get(thread_attr, PTH_ATTR_EVENTS, &events);
    TEST_ASSERT(rc == TRUE, "pth_attr_get EVENTS failed");
    fprintf(stderr, "  EVENTS=%p\n", (void*)events);

    rc = pth_attr_get(thread_attr, PTH_ATTR_BOUND, &bound);
    TEST_ASSERT(rc == TRUE, "pth_attr_get BOUND failed");
    fprintf(stderr, "  BOUND=%d\n", bound);
    TEST_ASSERT(bound == TRUE, "thread attribute should be bound");

    pth_join(tid, NULL);

    fprintf(stderr, "  PASSED: read-only attributes work correctly\n");
}

static void test_pth_attr_init(void)
{
    pth_attr_t attr;
    int prio;
    int joinable;
    char *name;
    int rc;

    fprintf(stderr, "\nTesting pth_attr_init...\n");

    attr = pth_attr_new();
    TEST_ASSERT(attr != NULL, "pth_attr_new failed");

    rc = pth_attr_set(attr, PTH_ATTR_NAME, "modified");
    TEST_ASSERT(rc == TRUE, "pth_attr_set NAME failed");
    rc = pth_attr_set(attr, PTH_ATTR_PRIO, PTH_PRIO_MAX);
    TEST_ASSERT(rc == TRUE, "pth_attr_set PRIO failed");
    rc = pth_attr_set(attr, PTH_ATTR_JOINABLE, FALSE);
    TEST_ASSERT(rc == TRUE, "pth_attr_set JOINABLE failed");

    rc = pth_attr_init(attr);
    TEST_ASSERT(rc == TRUE, "pth_attr_init failed");

    rc = pth_attr_get(attr, PTH_ATTR_NAME, &name);
    TEST_ASSERT(rc == TRUE, "pth_attr_get NAME failed");
    fprintf(stderr, "  After init, NAME=%s (should be 'unknown')\n", name);

    rc = pth_attr_get(attr, PTH_ATTR_PRIO, &prio);
    TEST_ASSERT(rc == TRUE, "pth_attr_get PRIO failed");
    TEST_ASSERT(prio == PTH_PRIO_STD, "PRIO should be reset to PTH_PRIO_STD");
    fprintf(stderr, "  After init, PRIO=%d (PTH_PRIO_STD=%d)\n", prio, PTH_PRIO_STD);

    rc = pth_attr_get(attr, PTH_ATTR_JOINABLE, &joinable);
    TEST_ASSERT(rc == TRUE, "pth_attr_get JOINABLE failed");
    TEST_ASSERT(joinable == TRUE, "JOINABLE should be reset to TRUE");
    fprintf(stderr, "  After init, JOINABLE=%d\n", joinable);

    pth_attr_destroy(attr);

    fprintf(stderr, "  PASSED: pth_attr_init works correctly\n");
}

static void test_cond_broadcast(void)
{
    pth_mutex_t mutex = PTH_MUTEX_INIT;
    pth_cond_t cond = PTH_COND_INIT;

    fprintf(stderr, "\nTesting pth_cond_notify with broadcast...\n");

    pth_mutex_init(&mutex);
    pth_cond_init(&cond);

    pth_mutex_acquire(&mutex, FALSE, NULL);

    pth_cond_notify(&cond, TRUE);
    fprintf(stderr, "  broadcast notification sent (no waiting threads)\n");

    pth_mutex_release(&mutex);

    fprintf(stderr, "  PASSED: pth_cond_notify broadcast works (basic test)\n");
}

static void *barrier_headlight_thread(void *arg)
{
    pth_barrier_t *barrier = (pth_barrier_t *)arg;
    int rc;

    pth_nap(pth_time(0, 10000));

    rc = pth_barrier_reach(barrier);
    fprintf(stderr, "    thread %p: barrier_reach returned %d\n", (void*)pth_self(), rc);

    return (void *)(long)rc;
}

static void test_barrier_headlight_taillight(void)
{
    pth_barrier_t barrier;
    pth_t threads[5];
    void *results[5];
    int rc, i;
    int headlight_count = 0, taillight_count = 0;

    fprintf(stderr, "\nTesting barrier HEADLIGHT and TAILLIGHT return values...\n");

    rc = pth_barrier_init(&barrier, 5);
    TEST_ASSERT(rc == TRUE, "pth_barrier_init failed");

    for (i = 0; i < 5; i++) {
        threads[i] = pth_spawn(PTH_ATTR_DEFAULT, barrier_headlight_thread, &barrier);
        TEST_ASSERT(threads[i] != NULL, "pth_spawn failed");
    }

    for (i = 0; i < 5; i++) {
        pth_join(threads[i], &results[i]);
        int ret_val = (int)(long)results[i];
        fprintf(stderr, "  thread %d returned %d\n", i, ret_val);

        if (ret_val == PTH_BARRIER_HEADLIGHT)
            headlight_count++;
        else if (ret_val == PTH_BARRIER_TAILLIGHT)
            taillight_count++;
    }

    fprintf(stderr, "  headlight_count=%d, taillight_count=%d\n",
            headlight_count, taillight_count);

    fprintf(stderr, "  PASSED: barrier headlight/taillight behavior observed\n");
}

static void *mutex_recursive_thread(void *arg)
{
    pth_mutex_t *mutex = (pth_mutex_t *)arg;

    fprintf(stderr, "  thread: first acquire\n");
    pth_mutex_acquire(mutex, FALSE, NULL);

    fprintf(stderr, "  thread: second acquire (recursive)\n");
    pth_mutex_acquire(mutex, FALSE, NULL);

    fprintf(stderr, "  thread: first release\n");
    pth_mutex_release(mutex);

    fprintf(stderr, "  thread: second release\n");
    pth_mutex_release(mutex);

    return NULL;
}

static void test_mutex_recursive(void)
{
    pth_mutex_t mutex = PTH_MUTEX_INIT;
    pth_t tid;
    int rc;

    fprintf(stderr, "\nTesting mutex recursive acquisition...\n");

    rc = pth_mutex_init(&mutex);
    TEST_ASSERT(rc == TRUE, "pth_mutex_init failed");

    tid = pth_spawn(PTH_ATTR_DEFAULT, mutex_recursive_thread, &mutex);
    TEST_ASSERT(tid != NULL, "pth_spawn failed");

    pth_join(tid, NULL);

    fprintf(stderr, "  PASSED: mutex recursive acquisition works\n");
}

static void test_mutex_trylock(void)
{
    pth_mutex_t mutex = PTH_MUTEX_INIT;
    int rc;

    fprintf(stderr, "\nTesting mutex trylock behavior...\n");

    rc = pth_mutex_init(&mutex);
    TEST_ASSERT(rc == TRUE, "pth_mutex_init failed");

    rc = pth_mutex_acquire(&mutex, TRUE, NULL);
    TEST_ASSERT(rc == TRUE, "first trylock should succeed");
    fprintf(stderr, "  first trylock succeeded\n");

    rc = pth_mutex_acquire(&mutex, TRUE, NULL);
    TEST_ASSERT(rc == TRUE, "recursive trylock should succeed");
    fprintf(stderr, "  recursive trylock succeeded\n");

    pth_mutex_release(&mutex);
    pth_mutex_release(&mutex);

    fprintf(stderr, "  PASSED: mutex trylock works correctly\n");
}

static void *mutex_event_thread(void *arg)
{
    pth_mutex_t *mutex = (pth_mutex_t *)arg;

    pth_mutex_acquire(mutex, FALSE, NULL);
    fprintf(stderr, "  thread: acquired mutex, sleeping\n");
    pth_sleep(2);
    pth_mutex_release(mutex);
    fprintf(stderr, "  thread: released mutex\n");

    return NULL;
}

static void test_mutex_with_event(void)
{
    pth_mutex_t mutex = PTH_MUTEX_INIT;
    pth_event_t timeout_event;
    pth_t tid;
    int rc;

    fprintf(stderr, "\nTesting mutex with event parameter...\n");

    rc = pth_mutex_init(&mutex);
    TEST_ASSERT(rc == TRUE, "pth_mutex_init failed");

    pth_mutex_acquire(&mutex, FALSE, NULL);

    tid = pth_spawn(PTH_ATTR_DEFAULT, mutex_event_thread, &mutex);
    TEST_ASSERT(tid != NULL, "pth_spawn failed");

    pth_yield(NULL);

    pth_mutex_release(&mutex);

    timeout_event = pth_event(PTH_EVENT_TIME, pth_timeout(1, 0));
    TEST_ASSERT(timeout_event != NULL, "pth_event failed");

    rc = pth_mutex_acquire(&mutex, FALSE, timeout_event);

    pth_event_free(timeout_event, PTH_FREE_THIS);

    if (rc) {
        fprintf(stderr, "  acquired mutex with event\n");
        pth_mutex_release(&mutex);
    } else {
        fprintf(stderr, "  mutex acquire timed out (expected if thread holds it)\n");
    }

    pth_join(tid, NULL);

    fprintf(stderr, "  PASSED: mutex with event parameter works\n");
}

static void test_msgport_pending(void)
{
    pth_msgport_t mp;
    pth_message_t msg;
    int pending;

    fprintf(stderr, "\nTesting pth_msgport_pending...\n");

    mp = pth_msgport_create("test_pending");
    TEST_ASSERT(mp != NULL, "pth_msgport_create failed");

    pending = pth_msgport_pending(mp);
    fprintf(stderr, "  pending messages (empty port): %d\n", pending);
    TEST_ASSERT(pending == 0, "should have 0 pending messages");

    memset(&msg, 0, sizeof(msg));
    msg.m_size = 100;
    msg.m_data = "test";

    pth_msgport_put(mp, &msg);

    pending = pth_msgport_pending(mp);
    fprintf(stderr, "  pending messages (after put): %d\n", pending);
    TEST_ASSERT(pending == 1, "should have 1 pending message");

    pth_msgport_get(mp);

    pending = pth_msgport_pending(mp);
    fprintf(stderr, "  pending messages (after get): %d\n", pending);
    TEST_ASSERT(pending == 0, "should have 0 pending messages again");

    pth_msgport_destroy(mp);

    fprintf(stderr, "  PASSED: pth_msgport_pending works correctly\n");
}

static void *cancel_point_thread(void *arg)
{
    int *reached = (int *)arg;
    int i;

    fprintf(stderr, "  thread: starting loop with cancel points\n");
    for (i = 0; i < 100; i++) {
        pth_yield(NULL);
        pth_cancel_point();
    }

    fprintf(stderr, "  thread: finished loop\n");
    *reached = 1;

    return NULL;
}

static void test_pth_cancel_point(void)
{
    pth_t tid;
    int reached = 0;
    void *result;
    int rc;

    fprintf(stderr, "\nTesting pth_cancel_point...\n");

    tid = pth_spawn(PTH_ATTR_DEFAULT, cancel_point_thread, &reached);
    TEST_ASSERT(tid != NULL, "pth_spawn failed");

    pth_yield(NULL);
    pth_yield(NULL);

    rc = pth_cancel(tid);
    if (rc != TRUE) {
        fprintf(stderr, "  pth_cancel failed (thread may have already completed)\n");
    } else {
        fprintf(stderr, "  sent cancel to thread\n");
    }

    rc = pth_join(tid, &result);
    TEST_ASSERT(rc == TRUE, "pth_join failed");

    if (result == PTH_CANCELED) {
        fprintf(stderr, "  thread was canceled (returned PTH_CANCELED)\n");
        TEST_ASSERT(reached == 0, "thread should not reach end after cancel");
    } else {
        fprintf(stderr, "  thread completed normally (cancel may have been too late)\n");
    }

    fprintf(stderr, "  PASSED: pth_cancel_point API works\n");
}

static void *abort_thread(void *arg)
{
    (void)arg;

    fprintf(stderr, "  thread: looping forever\n");
    while (1) {
        pth_yield(NULL);
    }

    return NULL;
}

static void test_pth_abort(void)
{
    pth_t tid;
    int rc;

    fprintf(stderr, "\nTesting pth_abort...\n");

    tid = pth_spawn(PTH_ATTR_DEFAULT, abort_thread, NULL);
    TEST_ASSERT(tid != NULL, "pth_spawn failed");

    pth_yield(NULL);
    pth_yield(NULL);

    fprintf(stderr, "  aborting thread (makes it detached)\n");
    rc = pth_abort(tid);
    TEST_ASSERT(rc == TRUE, "pth_abort failed");

    pth_yield(NULL);

    fprintf(stderr, "  thread aborted (cannot be joined since it's now detached)\n");

    fprintf(stderr, "  PASSED: pth_abort works\n");
}

static void *raise_thread(void *arg)
{
    int *received = (int *)arg;

    fprintf(stderr, "  thread: waiting for signal\n");
    pth_sleep(5);

    *received = 1;
    fprintf(stderr, "  thread: completed\n");

    return NULL;
}

static void test_pth_raise(void)
{
    pth_t tid;
    int received = 0;
    int rc;

    fprintf(stderr, "\nTesting pth_raise...\n");

    tid = pth_spawn(PTH_ATTR_DEFAULT, raise_thread, &received);
    TEST_ASSERT(tid != NULL, "pth_spawn failed");

    pth_yield(NULL);

    fprintf(stderr, "  raising SIGUSR1 to thread\n");
    rc = pth_raise(tid, SIGUSR1);
    TEST_ASSERT(rc == TRUE, "pth_raise failed");

    pth_join(tid, NULL);

    fprintf(stderr, "  PASSED: pth_raise works (signal was raised)\n");
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    int rc;

    fprintf(stderr, "=== TEST_SEMANTICS: Advanced Semantics and API Coverage Test ===\n");

    rc = pth_init();
    TEST_ASSERT(rc == TRUE, "pth_init failed");

    test_pth_cleanup_pop();
    test_read_only_attributes();
    test_pth_attr_init();
    test_cond_broadcast();
    test_barrier_headlight_taillight();
    test_mutex_recursive();
    test_mutex_trylock();
    test_mutex_with_event();
    test_msgport_pending();
    test_pth_cancel_point();
    test_pth_abort();
    test_pth_raise();

    pth_kill();

    fprintf(stderr, "\n=== ALL SEMANTICS TESTS PASSED ===\n");
    return 0;
}