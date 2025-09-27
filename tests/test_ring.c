/*
**  GNU Pth - The GNU Portable Threads
**  Copyright (c) 1999-2006 Ralf S. Engelschall <rse@engelschall.com>
**
**  Test: Ring buffer operations (pth_ring.c) - internal API test
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pth.h"
#include "pth_p.h"

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

typedef struct test_node_st {
    pth_ringnode_t node;
    int value;
} test_node_t;

static void test_ring_init(void)
{
    pth_ring_t ring;

    TEST("pth_ring_init: initialization");
    pth_ring_init(&ring);
    ASSERT(pth_ring_elements(&ring) == 0, "ring should be empty");
    ASSERT(pth_ring_first(&ring) == NULL, "first should be NULL");
    PASS();
}

static void test_ring_append_single(void)
{
    pth_ring_t ring;
    test_node_t node1;

    TEST("pth_ring_append: single node");
    pth_ring_init(&ring);
    node1.value = 1;

    pth_ring_append(&ring, &node1.node);
    ASSERT(pth_ring_elements(&ring) == 1, "ring should have 1 element");
    ASSERT(pth_ring_first(&ring) == &node1.node, "first should be node1");
    ASSERT(pth_ring_last(&ring) == &node1.node, "last should be node1");
    PASS();
}

static void test_ring_append_multiple(void)
{
    pth_ring_t ring;
    test_node_t node1, node2, node3;

    TEST("pth_ring_append: multiple nodes");
    pth_ring_init(&ring);
    node1.value = 1;
    node2.value = 2;
    node3.value = 3;

    pth_ring_append(&ring, &node1.node);
    pth_ring_append(&ring, &node2.node);
    pth_ring_append(&ring, &node3.node);

    ASSERT(pth_ring_elements(&ring) == 3, "ring should have 3 elements");
    ASSERT(pth_ring_first(&ring) == &node1.node, "first should be node1");
    ASSERT(pth_ring_last(&ring) == &node3.node, "last should be node3");
    PASS();
}

static void test_ring_prepend_single(void)
{
    pth_ring_t ring;
    test_node_t node1;

    TEST("pth_ring_prepend: single node");
    pth_ring_init(&ring);
    node1.value = 1;

    pth_ring_prepend(&ring, &node1.node);
    ASSERT(pth_ring_elements(&ring) == 1, "ring should have 1 element");
    ASSERT(pth_ring_first(&ring) == &node1.node, "first should be node1");
    PASS();
}

static void test_ring_prepend_multiple(void)
{
    pth_ring_t ring;
    test_node_t node1, node2, node3;

    TEST("pth_ring_prepend: multiple nodes");
    pth_ring_init(&ring);
    node1.value = 1;
    node2.value = 2;
    node3.value = 3;

    pth_ring_prepend(&ring, &node1.node);
    pth_ring_prepend(&ring, &node2.node);
    pth_ring_prepend(&ring, &node3.node);

    ASSERT(pth_ring_elements(&ring) == 3, "ring should have 3 elements");
    ASSERT(pth_ring_first(&ring) == &node3.node, "first should be node3");
    PASS();
}

static void test_ring_delete_single(void)
{
    pth_ring_t ring;
    test_node_t node1;

    TEST("pth_ring_delete: delete single node");
    pth_ring_init(&ring);
    node1.value = 1;

    pth_ring_append(&ring, &node1.node);
    pth_ring_delete(&ring, &node1.node);

    ASSERT(pth_ring_elements(&ring) == 0, "ring should be empty");
    ASSERT(pth_ring_first(&ring) == NULL, "first should be NULL");
    PASS();
}

static void test_ring_delete_middle(void)
{
    pth_ring_t ring;
    test_node_t node1, node2, node3;

    TEST("pth_ring_delete: delete middle node");
    pth_ring_init(&ring);
    node1.value = 1;
    node2.value = 2;
    node3.value = 3;

    pth_ring_append(&ring, &node1.node);
    pth_ring_append(&ring, &node2.node);
    pth_ring_append(&ring, &node3.node);

    pth_ring_delete(&ring, &node2.node);

    ASSERT(pth_ring_elements(&ring) == 2, "ring should have 2 elements");
    ASSERT(pth_ring_first(&ring) == &node1.node, "first should be node1");
    ASSERT(pth_ring_last(&ring) == &node3.node, "last should be node3");
    PASS();
}

static void test_ring_delete_first(void)
{
    pth_ring_t ring;
    test_node_t node1, node2, node3;

    TEST("pth_ring_delete: delete first node");
    pth_ring_init(&ring);
    node1.value = 1;
    node2.value = 2;
    node3.value = 3;

    pth_ring_append(&ring, &node1.node);
    pth_ring_append(&ring, &node2.node);
    pth_ring_append(&ring, &node3.node);

    pth_ring_delete(&ring, &node1.node);

    ASSERT(pth_ring_elements(&ring) == 2, "ring should have 2 elements");
    ASSERT(pth_ring_first(&ring) == &node2.node, "first should be node2");
    PASS();
}

static void test_ring_delete_last(void)
{
    pth_ring_t ring;
    test_node_t node1, node2, node3;

    TEST("pth_ring_delete: delete last node");
    pth_ring_init(&ring);
    node1.value = 1;
    node2.value = 2;
    node3.value = 3;

    pth_ring_append(&ring, &node1.node);
    pth_ring_append(&ring, &node2.node);
    pth_ring_append(&ring, &node3.node);

    pth_ring_delete(&ring, &node3.node);

    ASSERT(pth_ring_elements(&ring) == 2, "ring should have 2 elements");
    ASSERT(pth_ring_last(&ring) == &node2.node, "last should be node2");
    PASS();
}

static void test_ring_pop(void)
{
    pth_ring_t ring;
    test_node_t node1, node2, node3;
    pth_ringnode_t *popped;

    TEST("pth_ring_pop: stack operations");
    pth_ring_init(&ring);
    node1.value = 1;
    node2.value = 2;
    node3.value = 3;

    pth_ring_push(&ring, &node1.node);
    pth_ring_push(&ring, &node2.node);
    pth_ring_push(&ring, &node3.node);

    popped = pth_ring_pop(&ring);
    ASSERT(popped == &node3.node, "should pop node3");
    ASSERT(pth_ring_elements(&ring) == 2, "ring should have 2 elements");

    popped = pth_ring_pop(&ring);
    ASSERT(popped == &node2.node, "should pop node2");

    popped = pth_ring_pop(&ring);
    ASSERT(popped == &node1.node, "should pop node1");
    ASSERT(pth_ring_elements(&ring) == 0, "ring should be empty");
    PASS();
}

static void test_ring_dequeue(void)
{
    pth_ring_t ring;
    test_node_t node1, node2, node3;
    pth_ringnode_t *dequeued;

    TEST("pth_ring_dequeue: queue operations");
    pth_ring_init(&ring);
    node1.value = 1;
    node2.value = 2;
    node3.value = 3;

    pth_ring_enqueue(&ring, &node1.node);
    pth_ring_enqueue(&ring, &node2.node);
    pth_ring_enqueue(&ring, &node3.node);

    dequeued = pth_ring_dequeue(&ring);
    ASSERT(dequeued == &node1.node, "should dequeue node1");
    ASSERT(pth_ring_elements(&ring) == 2, "ring should have 2 elements");

    dequeued = pth_ring_dequeue(&ring);
    ASSERT(dequeued == &node2.node, "should dequeue node2");

    dequeued = pth_ring_dequeue(&ring);
    ASSERT(dequeued == &node3.node, "should dequeue node3");
    ASSERT(pth_ring_elements(&ring) == 0, "ring should be empty");
    PASS();
}

static void test_ring_contains(void)
{
    pth_ring_t ring;
    test_node_t node1, node2, node3, node4;

    TEST("pth_ring_contains: membership test");
    pth_ring_init(&ring);
    node1.value = 1;
    node2.value = 2;
    node3.value = 3;
    node4.value = 4;

    pth_ring_append(&ring, &node1.node);
    pth_ring_append(&ring, &node2.node);
    pth_ring_append(&ring, &node3.node);

    ASSERT(pth_ring_contains(&ring, &node1.node) == TRUE, "should contain node1");
    ASSERT(pth_ring_contains(&ring, &node2.node) == TRUE, "should contain node2");
    ASSERT(pth_ring_contains(&ring, &node3.node) == TRUE, "should contain node3");
    ASSERT(pth_ring_contains(&ring, &node4.node) == FALSE, "should not contain node4");
    PASS();
}

static void test_ring_favorite(void)
{
    pth_ring_t ring;
    test_node_t node1, node2, node3;
    int rc;

    TEST("pth_ring_favorite: make node favorite");
    pth_ring_init(&ring);
    node1.value = 1;
    node2.value = 2;
    node3.value = 3;

    pth_ring_append(&ring, &node1.node);
    pth_ring_append(&ring, &node2.node);
    pth_ring_append(&ring, &node3.node);

    rc = pth_ring_favorite(&ring, &node3.node);
    ASSERT(rc == TRUE, "favorite should succeed");
    ASSERT(pth_ring_first(&ring) == &node3.node, "node3 should be first");
    ASSERT(pth_ring_elements(&ring) == 3, "ring should still have 3 elements");
    PASS();
}

static void test_ring_insert_after(void)
{
    pth_ring_t ring;
    test_node_t node1, node2, node3;

    TEST("pth_ring_insert_after: insert node after another");
    pth_ring_init(&ring);
    node1.value = 1;
    node2.value = 2;
    node3.value = 3;

    pth_ring_append(&ring, &node1.node);
    pth_ring_append(&ring, &node3.node);

    pth_ring_insert_after(&ring, &node1.node, &node2.node);

    ASSERT(pth_ring_elements(&ring) == 3, "ring should have 3 elements");
    ASSERT(pth_ring_next(&ring, &node1.node) == &node2.node, "node2 should be after node1");
    ASSERT(pth_ring_next(&ring, &node2.node) == &node3.node, "node3 should be after node2");
    PASS();
}

static void test_ring_insert_before(void)
{
    pth_ring_t ring;
    test_node_t node1, node2, node3;

    TEST("pth_ring_insert_before: insert node before another");
    pth_ring_init(&ring);
    node1.value = 1;
    node2.value = 2;
    node3.value = 3;

    pth_ring_append(&ring, &node1.node);
    pth_ring_append(&ring, &node3.node);

    pth_ring_insert_before(&ring, &node3.node, &node2.node);

    ASSERT(pth_ring_elements(&ring) == 3, "ring should have 3 elements");
    ASSERT(pth_ring_next(&ring, &node1.node) == &node2.node, "node2 should be after node1");
    ASSERT(pth_ring_next(&ring, &node2.node) == &node3.node, "node3 should be after node2");
    PASS();
}

static void test_ring_walk(void)
{
    pth_ring_t ring;
    test_node_t node1, node2, node3;
    pth_ringnode_t *current;
    int count;

    TEST("pth_ring: walk through ring");
    pth_ring_init(&ring);
    node1.value = 1;
    node2.value = 2;
    node3.value = 3;

    pth_ring_append(&ring, &node1.node);
    pth_ring_append(&ring, &node2.node);
    pth_ring_append(&ring, &node3.node);

    count = 0;
    current = pth_ring_first(&ring);
    while (current != NULL) {
        count++;
        current = pth_ring_next(&ring, current);
    }

    ASSERT(count == 3, "should walk through 3 nodes");
    PASS();
}

static void test_ring_empty_operations(void)
{
    pth_ring_t ring;
    pth_ringnode_t *popped;

    TEST("pth_ring: operations on empty ring");
    pth_ring_init(&ring);

    popped = pth_ring_pop(&ring);
    ASSERT(popped == NULL, "pop on empty ring should return NULL");

    popped = pth_ring_dequeue(&ring);
    ASSERT(popped == NULL, "dequeue on empty ring should return NULL");

    PASS();
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
    printf("========================================\n");
    printf("Ring Buffer Operations Test Suite\n");
    printf("========================================\n\n");

    if (!pth_init()) {
        fprintf(stderr, "ERROR: pth_init() failed\n");
        return 1;
    }

    test_ring_init();
    test_ring_append_single();
    test_ring_append_multiple();
    test_ring_prepend_single();
    test_ring_prepend_multiple();
    test_ring_delete_single();
    test_ring_delete_middle();
    test_ring_delete_first();
    test_ring_delete_last();
    test_ring_pop();
    test_ring_dequeue();
    test_ring_contains();
    test_ring_favorite();
    test_ring_insert_after();
    test_ring_insert_before();
    test_ring_walk();
    test_ring_empty_operations();

    printf("\n========================================\n");
    printf("Test Results:\n");
    printf("  Total:  %d\n", test_count);
    printf("  Passed: %d\n", test_passed);
    printf("  Failed: %d\n", test_failed);
    printf("========================================\n");

    pth_kill();

    return (test_failed == 0) ? 0 : 1;
}