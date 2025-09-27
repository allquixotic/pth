/*
**  GNU Pth - The GNU Portable Threads
**  Copyright (c) 1999-2006 Ralf S. Engelschall <rse@engelschall.com>
**
**  Test: String formatting functions (pth_string.c)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "pth.h"
#include "pth_p.h"

#define TEST_BUFFER_SIZE 256

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

#define ASSERT_STR_EQ(actual, expected) do { \
    if (strcmp(actual, expected) != 0) { \
        printf("expected '%s', got '%s'", expected, actual); \
        FAIL("string mismatch"); \
        return; \
    } \
} while (0)

#define ASSERT_INT_EQ(actual, expected) do { \
    if ((actual) != (expected)) { \
        printf("expected %d, got %d", expected, actual); \
        FAIL("integer mismatch"); \
        return; \
    } \
} while (0)

static void test_snprintf_basic_string(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: basic string");
    n = pth_snprintf(buf, sizeof(buf), "Hello, %s!", "World");
    ASSERT_STR_EQ(buf, "Hello, World!");
    ASSERT_INT_EQ(n, 13);
    PASS();
}

static void test_snprintf_integer(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: integer formatting");
    n = pth_snprintf(buf, sizeof(buf), "Value: %d", 42);
    ASSERT_STR_EQ(buf, "Value: 42");
    ASSERT_INT_EQ(n, 9);
    PASS();
}

static void test_snprintf_negative_integer(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: negative integer");
    n = pth_snprintf(buf, sizeof(buf), "Neg: %d", -42);
    ASSERT_STR_EQ(buf, "Neg: -42");
    ASSERT_INT_EQ(n, 8);
    PASS();
}

static void test_snprintf_unsigned(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: unsigned integer");
    n = pth_snprintf(buf, sizeof(buf), "Unsigned: %u", 12345U);
    ASSERT_STR_EQ(buf, "Unsigned: 12345");
    ASSERT_INT_EQ(n, 15);
    PASS();
}

static void test_snprintf_hex_lower(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: hex lowercase");
    n = pth_snprintf(buf, sizeof(buf), "Hex: 0x%x", 0xABCD);
    ASSERT_STR_EQ(buf, "Hex: 0xabcd");
    ASSERT_INT_EQ(n, 11);
    PASS();
}

static void test_snprintf_hex_upper(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: hex uppercase");
    n = pth_snprintf(buf, sizeof(buf), "HEX: 0x%X", 0xABCD);
    ASSERT_STR_EQ(buf, "HEX: 0xABCD");
    ASSERT_INT_EQ(n, 11);
    PASS();
}

static void test_snprintf_octal(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: octal");
    n = pth_snprintf(buf, sizeof(buf), "Octal: %o", 0755);
    ASSERT_STR_EQ(buf, "Octal: 755");
    ASSERT_INT_EQ(n, 10);
    PASS();
}

static void test_snprintf_pointer(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));
    void *ptr = (void *)0x1234;

    TEST("pth_snprintf: pointer");
    n = pth_snprintf(buf, sizeof(buf), "Ptr: %p", ptr);
    ASSERT(strstr(buf, "1234") != NULL, "pointer not formatted correctly");
    ASSERT(n > 0, "return value should be positive");
    PASS();
}

static void test_snprintf_char(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: character");
    n = pth_snprintf(buf, sizeof(buf), "Char: %c", 'X');
    ASSERT_STR_EQ(buf, "Char: X");
    ASSERT_INT_EQ(n, 7);
    PASS();
}

static void test_snprintf_percent(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: percent sign");
    n = pth_snprintf(buf, sizeof(buf), "100%% done");
    ASSERT_STR_EQ(buf, "100% done");
    ASSERT_INT_EQ(n, 9);
    PASS();
}

static void test_snprintf_float(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: float");
    n = pth_snprintf(buf, sizeof(buf), "Pi: %.2f", 3.14159);
    ASSERT_STR_EQ(buf, "Pi: 3.14");
    ASSERT_INT_EQ(n, 8);
    PASS();
}

static void test_snprintf_float_precision(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: float with precision");
    n = pth_snprintf(buf, sizeof(buf), "Value: %.5f", 1.23456789);
    ASSERT_STR_EQ(buf, "Value: 1.23457");
    ASSERT_INT_EQ(n, 14);
    PASS();
}

static void test_snprintf_negative_float(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: negative float");
    n = pth_snprintf(buf, sizeof(buf), "Neg: %.2f", -3.14);
    ASSERT_STR_EQ(buf, "Neg: -3.14");
    ASSERT_INT_EQ(n, 10);
    PASS();
}

static void test_snprintf_width_integer(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: width for integer");
    n = pth_snprintf(buf, sizeof(buf), "Val: %5d", 42);
    ASSERT_STR_EQ(buf, "Val:    42");
    ASSERT_INT_EQ(n, 10);
    PASS();
}

static void test_snprintf_left_align(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: left align");
    n = pth_snprintf(buf, sizeof(buf), "Val: %-5d|", 42);
    ASSERT_STR_EQ(buf, "Val: 42   |");
    ASSERT_INT_EQ(n, 11);
    PASS();
}

static void test_snprintf_zero_pad(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: zero padding");
    n = pth_snprintf(buf, sizeof(buf), "Val: %05d", 42);
    ASSERT_STR_EQ(buf, "Val: 00042");
    ASSERT_INT_EQ(n, 10);
    PASS();
}

static void test_snprintf_plus_sign(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: plus sign");
    n = pth_snprintf(buf, sizeof(buf), "Val: %+d", 42);
    ASSERT_STR_EQ(buf, "Val: +42");
    ASSERT_INT_EQ(n, 8);
    PASS();
}

static void test_snprintf_space_sign(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: space sign");
    n = pth_snprintf(buf, sizeof(buf), "Val: % d", 42);
    ASSERT_STR_EQ(buf, "Val:  42");
    ASSERT_INT_EQ(n, 8);
    PASS();
}

static void test_snprintf_multiple_args(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: multiple arguments");
    n = pth_snprintf(buf, sizeof(buf), "%s: %d, %x, %c", "Test", 10, 0xFF, 'Z');
    ASSERT_STR_EQ(buf, "Test: 10, ff, Z");
    ASSERT_INT_EQ(n, 15);
    PASS();
}

static void test_snprintf_null_string(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: NULL string");
    n = pth_snprintf(buf, sizeof(buf), "Str: %s", (char *)NULL);
    ASSERT_STR_EQ(buf, "Str: <NULL>");
    ASSERT_INT_EQ(n, 11);
    PASS();
}

static void test_snprintf_truncation(void)
{
    char buf[10];
    int n __attribute__((unused));

    TEST("pth_snprintf: buffer truncation");
    n = pth_snprintf(buf, sizeof(buf), "This is a very long string");
    ASSERT(strlen(buf) < sizeof(buf), "buffer overflow");
    ASSERT(buf[sizeof(buf)-1] == '\0', "not null-terminated");
    ASSERT(n > 0, "return value should be positive");
    PASS();
}

static void test_snprintf_long_int(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: long integer");
    n = pth_snprintf(buf, sizeof(buf), "Long: %ld", 123456789L);
    ASSERT_STR_EQ(buf, "Long: 123456789");
    ASSERT_INT_EQ(n, 15);
    PASS();
}

static void test_snprintf_long_long_int(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: long long integer");
    n = pth_snprintf(buf, sizeof(buf), "LongLong: %lld", 9876543210LL);
    ASSERT_STR_EQ(buf, "LongLong: 9876543210");
    ASSERT_INT_EQ(n, 20);
    PASS();
}

static void test_snprintf_short_int(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));
    short val = 123;

    TEST("pth_snprintf: short integer");
    n = pth_snprintf(buf, sizeof(buf), "Short: %hd", val);
    ASSERT_STR_EQ(buf, "Short: 123");
    ASSERT_INT_EQ(n, 10);
    PASS();
}

static void test_snprintf_precision_string(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: string precision");
    n = pth_snprintf(buf, sizeof(buf), "Str: %.5s", "HelloWorld");
    ASSERT_STR_EQ(buf, "Str: Hello");
    ASSERT_INT_EQ(n, 10);
    PASS();
}

static void test_snprintf_width_string(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: string width");
    n = pth_snprintf(buf, sizeof(buf), "Str: %10s", "Hi");
    ASSERT_STR_EQ(buf, "Str:         Hi");
    ASSERT_INT_EQ(n, 15);
    PASS();
}

static void test_snprintf_empty_string(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: empty string");
    n = pth_snprintf(buf, sizeof(buf), "Empty: '%s'", "");
    ASSERT_STR_EQ(buf, "Empty: ''");
    ASSERT_INT_EQ(n, 9);
    PASS();
}

static void test_snprintf_zero(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: zero value");
    n = pth_snprintf(buf, sizeof(buf), "Zero: %d", 0);
    ASSERT_STR_EQ(buf, "Zero: 0");
    ASSERT_INT_EQ(n, 7);
    PASS();
}

static void test_snprintf_hash_flag(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: hash flag with octal");
    n = pth_snprintf(buf, sizeof(buf), "Octal: %#o", 0755);
    ASSERT(buf[0] != '\0', "buffer should not be empty");
    PASS();
}

static void test_snprintf_n_specifier(void)
{
    char buf[TEST_BUFFER_SIZE];
    int written = 0;
    int n __attribute__((unused));

    TEST("pth_snprintf: %n specifier");
    n = pth_snprintf(buf, sizeof(buf), "Hello%n World", &written);
    ASSERT_INT_EQ(written, 5);
    ASSERT_STR_EQ(buf, "Hello World");
    PASS();
}

static void test_vsnprintf_wrapper(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_vsnprintf: via wrapper");

    n = pth_snprintf(buf, sizeof(buf), "Format: %d %s %x", 42, "test", 0xAB);
    ASSERT_STR_EQ(buf, "Format: 42 test ab");
    ASSERT_INT_EQ(n, 18);
    PASS();
}

static void test_snprintf_complex_format(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));

    TEST("pth_snprintf: complex format");
    n = pth_snprintf(buf, sizeof(buf), "Int:%05d Hex:0x%04X Float:%.2f Str:%-10s Char:%c",
                     42, 0xAB, 3.14, "Test", 'Z');
    ASSERT(strlen(buf) > 0, "buffer should not be empty");
    ASSERT(strstr(buf, "00042") != NULL, "should contain zero-padded int");
    ASSERT(strstr(buf, "00AB") != NULL, "should contain zero-padded hex");
    ASSERT(strstr(buf, "3.14") != NULL, "should contain float");
    PASS();
}

static void test_snprintf_size_calculation(void)
{
    int n __attribute__((unused));

    TEST("pth_snprintf: size calculation with NULL buffer");
    n = pth_snprintf(NULL, 0, "This is a test: %d %s", 42, "hello");
    ASSERT(n > 0, "should return positive size");
    ASSERT(n > 20, "should be reasonable size");
    PASS();
}

static void test_snprintf_exact_size(void)
{
    char buf[12];
    int n __attribute__((unused));

    TEST("pth_snprintf: exact buffer size");
    n = pth_snprintf(buf, sizeof(buf), "Hello World");
    ASSERT_STR_EQ(buf, "Hello World");
    ASSERT_INT_EQ(n, 11);
    PASS();
}

static void test_snprintf_one_byte_buffer(void)
{
    char buf[1];
    int n __attribute__((unused));

    TEST("pth_snprintf: one byte buffer");
    n = pth_snprintf(buf, sizeof(buf), "Hello");
    ASSERT_STR_EQ(buf, "");
    PASS();
}

static void test_snprintf_mixed_types(void)
{
    char buf[TEST_BUFFER_SIZE];
    int n __attribute__((unused));
    long lval = 123456L;
    unsigned uval = 789U;

    TEST("pth_snprintf: mixed types");
    n = pth_snprintf(buf, sizeof(buf), "l:%ld u:%u d:%d", lval, uval, -50);
    ASSERT_STR_EQ(buf, "l:123456 u:789 d:-50");
    PASS();
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
    printf("========================================\n");
    printf("String Formatting Test Suite\n");
    printf("========================================\n\n");

    if (!pth_init()) {
        fprintf(stderr, "ERROR: pth_init() failed\n");
        return 1;
    }

    test_snprintf_basic_string();
    test_snprintf_integer();
    test_snprintf_negative_integer();
    test_snprintf_unsigned();
    test_snprintf_hex_lower();
    test_snprintf_hex_upper();
    test_snprintf_octal();
    test_snprintf_pointer();
    test_snprintf_char();
    test_snprintf_percent();
    test_snprintf_float();
    test_snprintf_float_precision();
    test_snprintf_negative_float();
    test_snprintf_width_integer();
    test_snprintf_left_align();
    test_snprintf_zero_pad();
    test_snprintf_plus_sign();
    test_snprintf_space_sign();
    test_snprintf_multiple_args();
    test_snprintf_null_string();
    test_snprintf_truncation();
    test_snprintf_long_int();
    test_snprintf_long_long_int();
    test_snprintf_short_int();
    test_snprintf_precision_string();
    test_snprintf_width_string();
    test_snprintf_empty_string();
    test_snprintf_zero();
    test_snprintf_hash_flag();
    test_snprintf_n_specifier();
    test_vsnprintf_wrapper();
    test_snprintf_complex_format();
    test_snprintf_size_calculation();
    test_snprintf_exact_size();
    test_snprintf_one_byte_buffer();
    test_snprintf_mixed_types();

    printf("\n========================================\n");
    printf("Test Results:\n");
    printf("  Total:  %d\n", test_count);
    printf("  Passed: %d\n", test_passed);
    printf("  Failed: %d\n", test_failed);
    printf("========================================\n");

    pth_kill();

    return (test_failed == 0) ? 0 : 1;
}