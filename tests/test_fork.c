/*
**  GNU Pth - The GNU Portable Threads
**  Copyright (c) 1999-2006 Ralf S. Engelschall <rse@engelschall.com>
**
**  Test: Fork handling (pth_fork.c)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "pth.h"

static int test_count = 0;
static int test_passed = 0;
static int test_failed = 0;

static int prepare_called = 0;
static int parent_called = 0;
static int child_called = 0;

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

static void prepare_handler(void *arg __attribute__((unused)))
{
    prepare_called++;
}

static void parent_handler(void *arg __attribute__((unused)))
{
    parent_called++;
}

static void child_handler(void *arg __attribute__((unused)))
{
    child_called++;
}

static void test_atfork_push_pop(void)
{
    int rc;

    TEST("pth_atfork_push/pop: basic registration");

    rc = pth_atfork_push(prepare_handler, parent_handler, child_handler, NULL);
    ASSERT(rc == TRUE, "atfork_push failed");

    rc = pth_atfork_pop();
    ASSERT(rc == TRUE, "atfork_pop failed");

    PASS();
}

static void test_atfork_push_multiple(void)
{
    int rc;

    TEST("pth_atfork_push: multiple handlers");

    rc = pth_atfork_push(prepare_handler, parent_handler, child_handler, NULL);
    ASSERT(rc == TRUE, "first atfork_push failed");

    rc = pth_atfork_push(prepare_handler, parent_handler, child_handler, NULL);
    ASSERT(rc == TRUE, "second atfork_push failed");

    rc = pth_atfork_pop();
    ASSERT(rc == TRUE, "first atfork_pop failed");

    rc = pth_atfork_pop();
    ASSERT(rc == TRUE, "second atfork_pop failed");

    PASS();
}

static void test_pth_fork_basic(void)
{
    pid_t pid;
    int status;

    TEST("pth_fork: basic fork");

    prepare_called = 0;
    parent_called = 0;
    child_called = 0;

    pth_atfork_push(prepare_handler, parent_handler, child_handler, NULL);

    pid = pth_fork();
    ASSERT(pid >= 0, "fork failed");

    if (pid == 0) {
        exit(0);
    } else {
        waitpid(pid, &status, 0);
        ASSERT(WIFEXITED(status), "child did not exit normally");
        ASSERT(WEXITSTATUS(status) == 0, "child exit status non-zero");
    }

    pth_atfork_pop();

    PASS();
}

static void test_pth_fork_handlers_called(void)
{
    pid_t pid;
    int status;

    TEST("pth_fork: handlers are called");

    prepare_called = 0;
    parent_called = 0;
    child_called = 0;

    pth_atfork_push(prepare_handler, parent_handler, child_handler, NULL);

    pid = pth_fork();
    ASSERT(pid >= 0, "fork failed");

    if (pid == 0) {
        exit(child_called > 0 ? 0 : 1);
    } else {
        waitpid(pid, &status, 0);
        ASSERT(prepare_called > 0, "prepare handler not called");
        ASSERT(parent_called > 0, "parent handler not called");
        ASSERT(WIFEXITED(status), "child did not exit normally");
        ASSERT(WEXITSTATUS(status) == 0, "child handler not called");
    }

    pth_atfork_pop();

    PASS();
}

static void test_pth_fork_thread_state(void)
{
    pid_t pid;
    int status;

    TEST("pth_fork: thread state after fork");

    pid = pth_fork();
    ASSERT(pid >= 0, "fork failed");

    if (pid == 0) {
        pth_t self = pth_self();
        exit(self != NULL ? 0 : 1);
    } else {
        waitpid(pid, &status, 0);
        ASSERT(WIFEXITED(status), "child did not exit normally");
        ASSERT(WEXITSTATUS(status) == 0, "child thread state invalid");
    }

    PASS();
}

static void test_pth_fork_no_handlers(void)
{
    pid_t pid;
    int status;

    TEST("pth_fork: fork without handlers");

    pid = pth_fork();
    ASSERT(pid >= 0, "fork failed");

    if (pid == 0) {
        exit(42);
    } else {
        waitpid(pid, &status, 0);
        ASSERT(WIFEXITED(status), "child did not exit normally");
        ASSERT(WEXITSTATUS(status) == 42, "child exit status incorrect");
    }

    PASS();
}

static void test_atfork_pop_empty(void)
{
    int rc;

    TEST("pth_atfork_pop: pop from empty stack");

    rc = pth_atfork_pop();
    ASSERT(rc == FALSE, "pop should fail on empty stack");

    PASS();
}

static void test_pth_fork_child_can_spawn(void)
{
    pid_t pid;
    int status;

    TEST("pth_fork: child can spawn threads");

    pid = pth_fork();
    ASSERT(pid >= 0, "fork failed");

    if (pid == 0) {
        exit(0);
    } else {
        waitpid(pid, &status, 0);
        ASSERT(WIFEXITED(status), "child did not exit normally");
        ASSERT(WEXITSTATUS(status) == 0, "child could not spawn thread");
    }

    PASS();
}

static void *simple_thread(void *arg)
{
    return arg;
}

static void test_pth_fork_with_running_threads(void)
{
    pid_t pid;
    int status;
    pth_attr_t attr;
    pth_t tid;

    TEST("pth_fork: fork with running threads");

    attr = pth_attr_new();
    pth_attr_set(attr, PTH_ATTR_JOINABLE, FALSE);
    tid = pth_spawn(attr, simple_thread, NULL);
    pth_attr_destroy(attr);
    ASSERT(tid != NULL, "thread spawn failed");

    pth_yield(NULL);

    pid = pth_fork();
    ASSERT(pid >= 0, "fork failed");

    if (pid == 0) {
        exit(0);
    } else {
        waitpid(pid, &status, 0);
        ASSERT(WIFEXITED(status), "child did not exit normally");
        ASSERT(WEXITSTATUS(status) == 0, "child exit status non-zero");
    }

    PASS();
}

static void test_atfork_null_handlers(void)
{
    int rc;

    TEST("pth_atfork_push: NULL handlers allowed");

    rc = pth_atfork_push(NULL, NULL, NULL, NULL);
    ASSERT(rc == TRUE, "atfork_push with NULL handlers failed");

    rc = pth_atfork_pop();
    ASSERT(rc == TRUE, "atfork_pop failed");

    PASS();
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
    printf("========================================\n");
    printf("Fork Handling Test Suite\n");
    printf("========================================\n\n");

    if (!pth_init()) {
        fprintf(stderr, "ERROR: pth_init() failed\n");
        return 1;
    }

    test_atfork_push_pop();
    test_atfork_push_multiple();
    test_pth_fork_basic();
    test_pth_fork_handlers_called();
    test_pth_fork_thread_state();
    test_pth_fork_no_handlers();
    test_atfork_pop_empty();
    test_pth_fork_child_can_spawn();
    test_pth_fork_with_running_threads();
    test_atfork_null_handlers();

    printf("\n========================================\n");
    printf("Test Results:\n");
    printf("  Total:  %d\n", test_count);
    printf("  Passed: %d\n", test_passed);
    printf("  Failed: %d\n", test_failed);
    printf("========================================\n");

    pth_kill();

    return (test_failed == 0) ? 0 : 1;
}