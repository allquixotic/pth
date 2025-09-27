/*
**  GNU Pth - The GNU Portable Threads
**  Copyright (c) 1999-2006 Ralf S. Engelschall <rse@engelschall.com>
**
**  This file is part of GNU Pth, a non-preemptive thread scheduling
**  library which can be found at http://www.gnu.org/software/pth/.
**
**  test_coverage.c: Comprehensive API coverage test
**  Tests APIs not covered by existing tests
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

static int once_counter = 0;
static pth_key_t test_key1, test_key2;
static pth_t main_thread;

static void once_func(void *arg)
{
    int *counter = (int *)arg;
    (*counter)++;
    fprintf(stderr, "  once_func called, counter=%d\n", *counter);
}

static void key_destructor(void *data)
{
    fprintf(stderr, "  key_destructor called with data=%p\n", data);
    free(data);
}

static void *test_once_thread(void *arg)
{
    static pth_once_t once = PTH_ONCE_INIT;
    (void)arg;

    pth_once(&once, once_func, &once_counter);
    pth_once(&once, once_func, &once_counter);

    return NULL;
}

static void test_pth_once(void)
{
    pth_t tid1, tid2, tid3;
    static pth_once_t once = PTH_ONCE_INIT;

    fprintf(stderr, "\nTesting pth_once...\n");

    once_counter = 0;
    pth_once(&once, once_func, &once_counter);
    TEST_ASSERT(once_counter == 1, "pth_once should call function once");

    pth_once(&once, once_func, &once_counter);
    TEST_ASSERT(once_counter == 1, "pth_once should not call function second time");

    tid1 = pth_spawn(PTH_ATTR_DEFAULT, test_once_thread, NULL);
    tid2 = pth_spawn(PTH_ATTR_DEFAULT, test_once_thread, NULL);
    tid3 = pth_spawn(PTH_ATTR_DEFAULT, test_once_thread, NULL);
    TEST_ASSERT(tid1 && tid2 && tid3, "pth_spawn failed");

    pth_join(tid1, NULL);
    pth_join(tid2, NULL);
    pth_join(tid3, NULL);

    fprintf(stderr, "  PASSED: pth_once works correctly\n");
}

static void *test_self_thread(void *arg)
{
    pth_t *expected = (pth_t *)arg;
    pth_t self = pth_self();

    fprintf(stderr, "  thread self=%p, expected=%p\n", (void*)self, (void*)*expected);
    TEST_ASSERT(self == *expected, "pth_self returned wrong thread");

    return NULL;
}

static void test_pth_self(void)
{
    pth_t tid;
    pth_t self;

    fprintf(stderr, "\nTesting pth_self...\n");

    self = pth_self();
    TEST_ASSERT(self != NULL, "pth_self returned NULL");
    fprintf(stderr, "  main thread self=%p\n", (void*)self);
    main_thread = self;

    tid = pth_spawn(PTH_ATTR_DEFAULT, test_self_thread, &tid);
    TEST_ASSERT(tid != NULL, "pth_spawn failed");

    pth_join(tid, NULL);

    fprintf(stderr, "  PASSED: pth_self works correctly\n");
}

static void *test_suspend_thread(void *arg)
{
    int *counter = (int *)arg;
    int i;

    for (i = 0; i < 10; i++) {
        (*counter)++;
        pth_yield(NULL);
    }

    return NULL;
}

static void test_pth_suspend_resume(void)
{
    pth_t tid;
    int counter = 0;
    int rc;

    fprintf(stderr, "\nTesting pth_suspend and pth_resume...\n");

    tid = pth_spawn(PTH_ATTR_DEFAULT, test_suspend_thread, &counter);
    TEST_ASSERT(tid != NULL, "pth_spawn failed");

    pth_yield(NULL);
    pth_yield(NULL);

    fprintf(stderr, "  counter before suspend=%d\n", counter);
    int counter_before_suspend = counter;

    rc = pth_suspend(tid);
    TEST_ASSERT(rc == TRUE, "pth_suspend failed");

    pth_yield(NULL);
    pth_yield(NULL);
    pth_yield(NULL);

    fprintf(stderr, "  counter after suspend=%d\n", counter);
    TEST_ASSERT(counter == counter_before_suspend, "thread continued after suspend");

    rc = pth_resume(tid);
    TEST_ASSERT(rc == TRUE, "pth_resume failed");

    pth_join(tid, NULL);

    fprintf(stderr, "  final counter=%d\n", counter);
    TEST_ASSERT(counter == 10, "thread did not complete after resume");

    fprintf(stderr, "  PASSED: pth_suspend and pth_resume work correctly\n");
}

static void test_pth_nap(void)
{
    pth_time_t duration;
    int rc;

    fprintf(stderr, "\nTesting pth_nap...\n");

    duration = pth_time(0, 100000);

    rc = pth_nap(duration);
    TEST_ASSERT(rc == TRUE, "pth_nap failed");

    fprintf(stderr, "  PASSED: pth_nap works correctly\n");
}

static void *test_exit_thread(void *arg)
{
    int *value = (int *)arg;

    fprintf(stderr, "  thread started\n");
    pth_exit(value);

    TEST_FAILED("thread continued after pth_exit");
    return NULL;
}

static void test_pth_exit(void)
{
    pth_t tid;
    int value = 42;
    void *result;
    int rc;

    fprintf(stderr, "\nTesting pth_exit...\n");

    tid = pth_spawn(PTH_ATTR_DEFAULT, test_exit_thread, &value);
    TEST_ASSERT(tid != NULL, "pth_spawn failed");

    rc = pth_join(tid, &result);
    TEST_ASSERT(rc == TRUE, "pth_join failed");
    TEST_ASSERT(result == &value, "pth_exit value not returned correctly");

    fprintf(stderr, "  PASSED: pth_exit works correctly\n");
}

static void test_pth_cancel_state(void)
{
    int old_state;

    fprintf(stderr, "\nTesting pth_cancel_state...\n");

    pth_cancel_state(PTH_CANCEL_DISABLE, &old_state);
    fprintf(stderr, "  disabled cancellation, old_state=%d\n", old_state);

    pth_cancel_state(PTH_CANCEL_ENABLE, NULL);
    fprintf(stderr, "  enabled cancellation\n");

    pth_cancel_state(PTH_CANCEL_ASYNCHRONOUS, &old_state);
    fprintf(stderr, "  set asynchronous, old_state=%d\n", old_state);

    pth_cancel_state(PTH_CANCEL_DEFERRED, NULL);
    fprintf(stderr, "  set deferred\n");

    fprintf(stderr, "  PASSED: pth_cancel_state works correctly\n");
}

static void test_pth_key_storage(void)
{
    int rc;
    void *data1, *data2;
    char *str1, *str2;

    fprintf(stderr, "\nTesting pth_key_* (key-based storage)...\n");

    rc = pth_key_create(&test_key1, key_destructor);
    TEST_ASSERT(rc == TRUE, "pth_key_create failed for key1");
    fprintf(stderr, "  created key1=%d\n", test_key1);

    rc = pth_key_create(&test_key2, NULL);
    TEST_ASSERT(rc == TRUE, "pth_key_create failed for key2");
    fprintf(stderr, "  created key2=%d\n", test_key2);

    str1 = (char *)malloc(32);
    strcpy(str1, "test data 1");
    rc = pth_key_setdata(test_key1, str1);
    TEST_ASSERT(rc == TRUE, "pth_key_setdata failed for key1");

    str2 = (char *)malloc(32);
    strcpy(str2, "test data 2");
    rc = pth_key_setdata(test_key2, str2);
    TEST_ASSERT(rc == TRUE, "pth_key_setdata failed for key2");

    data1 = pth_key_getdata(test_key1);
    TEST_ASSERT(data1 == str1, "pth_key_getdata returned wrong data for key1");
    TEST_ASSERT(strcmp((char *)data1, "test data 1") == 0, "key1 data corrupted");
    fprintf(stderr, "  key1 data: %s\n", (char *)data1);

    data2 = pth_key_getdata(test_key2);
    TEST_ASSERT(data2 == str2, "pth_key_getdata returned wrong data for key2");
    TEST_ASSERT(strcmp((char *)data2, "test data 2") == 0, "key2 data corrupted");
    fprintf(stderr, "  key2 data: %s\n", (char *)data2);

    rc = pth_key_setdata(test_key1, NULL);
    TEST_ASSERT(rc == TRUE, "pth_key_setdata NULL failed");

    data1 = pth_key_getdata(test_key1);
    TEST_ASSERT(data1 == NULL, "pth_key_getdata should return NULL after setting NULL");

    rc = pth_key_setdata(test_key2, NULL);
    TEST_ASSERT(rc == TRUE, "pth_key_setdata NULL failed for key2");

    free(str1);
    free(str2);

    rc = pth_key_delete(test_key1);
    TEST_ASSERT(rc == TRUE, "pth_key_delete failed for key1");

    rc = pth_key_delete(test_key2);
    TEST_ASSERT(rc == TRUE, "pth_key_delete failed for key2");

    fprintf(stderr, "  PASSED: pth_key_* works correctly\n");
}

static void *rwlock_reader_thread(void *arg)
{
    pth_rwlock_t *rwlock = (pth_rwlock_t *)arg;
    int i;

    for (i = 0; i < 5; i++) {
        pth_rwlock_acquire(rwlock, PTH_RWLOCK_RD, FALSE, NULL);
        fprintf(stderr, "    reader acquired lock (iteration %d)\n", i);
        pth_nap(pth_time(0, 10000));
        pth_rwlock_release(rwlock);
        pth_yield(NULL);
    }

    return NULL;
}

static void *rwlock_writer_thread(void *arg)
{
    pth_rwlock_t *rwlock = (pth_rwlock_t *)arg;
    int i;

    for (i = 0; i < 3; i++) {
        pth_rwlock_acquire(rwlock, PTH_RWLOCK_RW, FALSE, NULL);
        fprintf(stderr, "    writer acquired lock (iteration %d)\n", i);
        pth_nap(pth_time(0, 20000));
        pth_rwlock_release(rwlock);
        pth_yield(NULL);
    }

    return NULL;
}

static void test_pth_rwlock(void)
{
    pth_rwlock_t rwlock = PTH_RWLOCK_INIT;
    pth_t readers[3], writers[2];
    int rc, i;

    fprintf(stderr, "\nTesting pth_rwlock_* (read-write locks)...\n");

    rc = pth_rwlock_init(&rwlock);
    TEST_ASSERT(rc == TRUE, "pth_rwlock_init failed");

    for (i = 0; i < 3; i++) {
        readers[i] = pth_spawn(PTH_ATTR_DEFAULT, rwlock_reader_thread, &rwlock);
        TEST_ASSERT(readers[i] != NULL, "pth_spawn failed for reader");
    }

    for (i = 0; i < 2; i++) {
        writers[i] = pth_spawn(PTH_ATTR_DEFAULT, rwlock_writer_thread, &rwlock);
        TEST_ASSERT(writers[i] != NULL, "pth_spawn failed for writer");
    }

    for (i = 0; i < 3; i++)
        pth_join(readers[i], NULL);
    for (i = 0; i < 2; i++)
        pth_join(writers[i], NULL);

    fprintf(stderr, "  PASSED: pth_rwlock_* works correctly\n");
}

static void *barrier_thread(void *arg)
{
    pth_barrier_t *barrier = (pth_barrier_t *)arg;
    pth_t self = pth_self();

    fprintf(stderr, "    thread %p: before barrier\n", (void*)self);
    pth_nap(pth_time(0, (rand() % 100) * 1000));

    int rc = pth_barrier_reach(barrier);

    fprintf(stderr, "    thread %p: after barrier (rc=%d)\n", (void*)self, rc);

    return NULL;
}

static void test_pth_barrier(void)
{
    pth_barrier_t barrier;
    pth_t threads[5];
    int rc, i;

    fprintf(stderr, "\nTesting pth_barrier_* (barriers)...\n");

    rc = pth_barrier_init(&barrier, 5);
    TEST_ASSERT(rc == TRUE, "pth_barrier_init failed");

    for (i = 0; i < 5; i++) {
        threads[i] = pth_spawn(PTH_ATTR_DEFAULT, barrier_thread, &barrier);
        TEST_ASSERT(threads[i] != NULL, "pth_spawn failed");
    }

    for (i = 0; i < 5; i++)
        pth_join(threads[i], NULL);

    fprintf(stderr, "  PASSED: pth_barrier_* works correctly\n");
}

static void test_pth_fdmode(void)
{
    int mode;
    int fds[2];

    fprintf(stderr, "\nTesting pth_fdmode...\n");

    if (pipe(fds) != 0)
        TEST_FAILED("pipe creation failed");

    mode = pth_fdmode(fds[0], PTH_FDMODE_POLL);
    fprintf(stderr, "  fd %d original mode=%d\n", fds[0], mode);

    mode = pth_fdmode(fds[0], PTH_FDMODE_NONBLOCK);
    TEST_ASSERT(mode != PTH_FDMODE_ERROR, "pth_fdmode set to NONBLOCK failed");
    fprintf(stderr, "  fd %d set to NONBLOCK, returned mode=%d\n", fds[0], mode);

    mode = pth_fdmode(fds[0], PTH_FDMODE_POLL);
    fprintf(stderr, "  fd %d current mode=%d\n", fds[0], mode);

    mode = pth_fdmode(fds[0], PTH_FDMODE_BLOCK);
    TEST_ASSERT(mode != PTH_FDMODE_ERROR, "pth_fdmode set to BLOCK failed");
    fprintf(stderr, "  fd %d set to BLOCK, returned mode=%d\n", fds[0], mode);

    close(fds[0]);
    close(fds[1]);

    fprintf(stderr, "  PASSED: pth_fdmode works correctly\n");
}

static void test_pth_time_timeout(void)
{
    pth_time_t t1, t2, t3;

    fprintf(stderr, "\nTesting pth_time and pth_timeout...\n");

    t1 = pth_time(5, 123456);
    fprintf(stderr, "  created time: tv_sec=5, tv_usec=123456\n");
    TEST_ASSERT(t1.tv_sec == 5, "pth_time tv_sec incorrect");
    TEST_ASSERT(t1.tv_usec == 123456, "pth_time tv_usec incorrect");

    t2 = pth_time(0, 0);
    fprintf(stderr, "  current time: tv_sec=%ld, tv_usec=%ld\n",
            (long)t2.tv_sec, (long)t2.tv_usec);

    t3 = pth_timeout(2, 500000);
    fprintf(stderr, "  timeout (2.5s from now): tv_sec=%ld, tv_usec=%ld\n",
            (long)t3.tv_sec, (long)t3.tv_usec);
    TEST_ASSERT(t3.tv_sec >= t2.tv_sec + 2, "pth_timeout tv_sec too small");

    fprintf(stderr, "  PASSED: pth_time and pth_timeout work correctly\n");
}

static void test_pth_attr_get(void)
{
    pth_attr_t attr;
    int prio, joinable;
    unsigned int stack_size;
    char *name;
    int rc;

    fprintf(stderr, "\nTesting pth_attr_get...\n");

    attr = pth_attr_new();
    TEST_ASSERT(attr != NULL, "pth_attr_new failed");

    rc = pth_attr_set(attr, PTH_ATTR_NAME, "test");
    TEST_ASSERT(rc == TRUE, "pth_attr_set NAME failed");

    rc = pth_attr_set(attr, PTH_ATTR_PRIO, PTH_PRIO_MAX);
    TEST_ASSERT(rc == TRUE, "pth_attr_set PRIO failed");

    rc = pth_attr_set(attr, PTH_ATTR_JOINABLE, FALSE);
    TEST_ASSERT(rc == TRUE, "pth_attr_set JOINABLE failed");

    rc = pth_attr_set(attr, PTH_ATTR_STACK_SIZE, 65536);
    TEST_ASSERT(rc == TRUE, "pth_attr_set STACK_SIZE failed");

    rc = pth_attr_get(attr, PTH_ATTR_NAME, &name);
    TEST_ASSERT(rc == TRUE, "pth_attr_get NAME failed");
    TEST_ASSERT(strcmp(name, "test") == 0, "NAME value incorrect");
    fprintf(stderr, "  NAME=%s\n", name);

    rc = pth_attr_get(attr, PTH_ATTR_PRIO, &prio);
    TEST_ASSERT(rc == TRUE, "pth_attr_get PRIO failed");
    TEST_ASSERT(prio == PTH_PRIO_MAX, "PRIO value incorrect");
    fprintf(stderr, "  PRIO=%d\n", prio);

    rc = pth_attr_get(attr, PTH_ATTR_JOINABLE, &joinable);
    TEST_ASSERT(rc == TRUE, "pth_attr_get JOINABLE failed");
    TEST_ASSERT(joinable == FALSE, "JOINABLE value incorrect");
    fprintf(stderr, "  JOINABLE=%d\n", joinable);

    rc = pth_attr_get(attr, PTH_ATTR_STACK_SIZE, &stack_size);
    TEST_ASSERT(rc == TRUE, "pth_attr_get STACK_SIZE failed");
    TEST_ASSERT(stack_size == 65536, "STACK_SIZE value incorrect");
    fprintf(stderr, "  STACK_SIZE=%u\n", stack_size);

    pth_attr_destroy(attr);

    fprintf(stderr, "  PASSED: pth_attr_get works correctly\n");
}

static void test_pth_ctrl_advanced(void)
{
    long result;

    fprintf(stderr, "\nTesting pth_ctrl with advanced flags...\n");

    result = pth_ctrl(PTH_CTRL_GETAVLOAD);
    fprintf(stderr, "  PTH_CTRL_GETAVLOAD=%ld\n", result);

    result = pth_ctrl(PTH_CTRL_GETTHREADS_NEW);
    fprintf(stderr, "  PTH_CTRL_GETTHREADS_NEW=%ld\n", result);

    result = pth_ctrl(PTH_CTRL_GETTHREADS_READY);
    fprintf(stderr, "  PTH_CTRL_GETTHREADS_READY=%ld\n", result);

    result = pth_ctrl(PTH_CTRL_GETTHREADS_RUNNING);
    fprintf(stderr, "  PTH_CTRL_GETTHREADS_RUNNING=%ld\n", result);

    result = pth_ctrl(PTH_CTRL_GETTHREADS_WAITING);
    fprintf(stderr, "  PTH_CTRL_GETTHREADS_WAITING=%ld\n", result);

    result = pth_ctrl(PTH_CTRL_GETTHREADS_SUSPENDED);
    fprintf(stderr, "  PTH_CTRL_GETTHREADS_SUSPENDED=%ld\n", result);

    result = pth_ctrl(PTH_CTRL_GETTHREADS_DEAD);
    fprintf(stderr, "  PTH_CTRL_GETTHREADS_DEAD=%ld\n", result);

    pth_ctrl(PTH_CTRL_FAVOURNEW, TRUE);
    fprintf(stderr, "  PTH_CTRL_FAVOURNEW set to TRUE\n");

    pth_ctrl(PTH_CTRL_FAVOURNEW, FALSE);
    fprintf(stderr, "  PTH_CTRL_FAVOURNEW set to FALSE\n");

    fprintf(stderr, "  PASSED: pth_ctrl advanced flags work\n");
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    int rc;

    fprintf(stderr, "=== TEST_COVERAGE: Comprehensive API Coverage Test ===\n");

    rc = pth_init();
    TEST_ASSERT(rc == TRUE, "pth_init failed");

    test_pth_once();
    test_pth_self();
    test_pth_suspend_resume();
    test_pth_nap();
    test_pth_exit();
    test_pth_cancel_state();
    test_pth_key_storage();
    test_pth_rwlock();
    test_pth_barrier();
    test_pth_fdmode();
    test_pth_time_timeout();
    test_pth_attr_get();
    test_pth_ctrl_advanced();

    pth_kill();

    fprintf(stderr, "\n=== ALL TESTS PASSED ===\n");
    return 0;
}