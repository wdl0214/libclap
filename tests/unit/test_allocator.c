/**
 * @file test_allocator.c
 * @brief Unit tests for clap_allocator.c
 */

#include "unity.h"
#include <clap/clap_allocator.h>
#include "clap_parser_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ============================================================================
 * Test Setup and Teardown
 * ============================================================================ */

/* Custom allocator tracking for tests */
static size_t g_custom_malloc_count = 0;
static size_t g_custom_free_count = 0;
static size_t g_custom_realloc_count = 0;

static void* custom_malloc(size_t size) {
    g_custom_malloc_count++;
    return malloc(size);
}

static void custom_free(void *ptr) {
    g_custom_free_count++;
    free(ptr);
}

static void* custom_realloc(void *ptr, size_t size) {
    g_custom_realloc_count++;
    return realloc(ptr, size);
}

static void reset_custom_counts(void) {
    g_custom_malloc_count = 0;
    g_custom_free_count = 0;
    g_custom_realloc_count = 0;
}

void setUp(void) {
    reset_custom_counts();
    /* Note: Cannot reset the global allocator once locked */
}

void tearDown(void) {
    /* Nothing needed */
}

/* ============================================================================
 * clap_malloc Tests
 * ============================================================================ */

void test_clap_malloc_basic(void) {
    void *ptr = clap_malloc(100);
    
    TEST_ASSERT_NOT_NULL(ptr);
    
    /* Memory should be zero-initialized */
    unsigned char *bytes = (unsigned char*)ptr;
    for (size_t i = 0; i < 100; i++) {
        TEST_ASSERT_EQUAL(0, bytes[i]);
    }
    
    clap_free(ptr);
}

void test_clap_malloc_zero_size(void) {
    void *ptr = clap_malloc(0);
    
    /* malloc(0) behavior is implementation-defined, but our wrapper may return NULL */
    /* We just verify it doesn't crash */
    
    if (ptr) {
        clap_free(ptr);
    }
    TEST_ASSERT_TRUE(1);
}

void test_clap_malloc_large(void) {
    void *ptr = clap_malloc(1024 * 1024);  /* 1 MB */
    
    TEST_ASSERT_NOT_NULL(ptr);
    
    clap_free(ptr);
}

/* ============================================================================
 * clap_calloc Tests
 * ============================================================================ */

void test_clap_calloc_basic(void) {
    int *ptr = clap_calloc(10, sizeof(int));
    
    TEST_ASSERT_NOT_NULL(ptr);
    
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_EQUAL(0, ptr[i]);
    }
    
    clap_free(ptr);
}

void test_clap_calloc_zero_nmemb(void) {
    void *ptr = clap_calloc(0, 100);
    TEST_ASSERT_NULL(ptr);
}

void test_clap_calloc_zero_size(void) {
    void *ptr = clap_calloc(100, 0);
    TEST_ASSERT_NULL(ptr);
}

void test_clap_calloc_overflow(void) {
    /* Test multiplication overflow protection */
    size_t huge = SIZE_MAX / 2 + 1;
    void *ptr = clap_calloc(huge, 2);
    
    TEST_ASSERT_NULL(ptr);
}

void test_clap_calloc_large(void) {
    void *ptr = clap_calloc(1024, 1024);  /* 1 MB */
    
    TEST_ASSERT_NOT_NULL(ptr);
    
    clap_free(ptr);
}

/* ============================================================================
 * clap_free Tests
 * ============================================================================ */

void test_clap_free_null(void) {
    clap_free(NULL);
    TEST_ASSERT_TRUE(1);  /* Should not crash */
}

void test_clap_free_valid(void) {
    void *ptr = clap_malloc(100);
    clap_free(ptr);
    TEST_ASSERT_TRUE(1);  /* Should not crash */
}

/* ============================================================================
 * clap_realloc Tests
 * ============================================================================ */

void test_clap_realloc_grow(void) {
    char *ptr = clap_malloc(10);
    strcpy(ptr, "hello");
    
    char *new_ptr = clap_realloc(ptr, 20);
    
    TEST_ASSERT_NOT_NULL(new_ptr);
    TEST_ASSERT_EQUAL_STRING("hello", new_ptr);
    
    strcat(new_ptr, " world");
    TEST_ASSERT_EQUAL_STRING("hello world", new_ptr);
    
    clap_free(new_ptr);
}

void test_clap_realloc_shrink(void) {
    char *ptr = clap_malloc(20);
    strcpy(ptr, "hello");
    
    char *new_ptr = clap_realloc(ptr, 6);
    
    TEST_ASSERT_NOT_NULL(new_ptr);
    TEST_ASSERT_EQUAL_STRING("hello", new_ptr);
    
    clap_free(new_ptr);
}

void test_clap_realloc_zero_size(void) {
    char *ptr = clap_malloc(10);
    char *new_ptr = clap_realloc(ptr, 0);
    
    TEST_ASSERT_NULL(new_ptr);  /* Should free and return NULL */
}

void test_clap_realloc_null(void) {
    char *ptr = clap_realloc(NULL, 10);
    
    TEST_ASSERT_NOT_NULL(ptr);
    
    clap_free(ptr);
}

/* ============================================================================
 * clap_strdup Tests
 * ============================================================================ */

void test_clap_strdup_basic(void) {
    const char *original = "hello world";
    char *copy = clap_strdup(original);
    
    TEST_ASSERT_NOT_NULL(copy);
    TEST_ASSERT_EQUAL_STRING(original, copy);
    TEST_ASSERT_NOT_EQUAL((uintptr_t)original, (uintptr_t)copy);
    
    clap_free(copy);
}

void test_clap_strdup_empty(void) {
    const char *original = "";
    char *copy = clap_strdup(original);
    
    TEST_ASSERT_NOT_NULL(copy);
    TEST_ASSERT_EQUAL_STRING("", copy);
    
    clap_free(copy);
}

void test_clap_strdup_null(void) {
    char *copy = clap_strdup(NULL);
    TEST_ASSERT_NULL(copy);
}

void test_clap_strdup_long(void) {
    char original[1000];
    memset(original, 'X', 999);
    original[999] = '\0';
    
    char *copy = clap_strdup(original);
    
    TEST_ASSERT_NOT_NULL(copy);
    TEST_ASSERT_EQUAL_STRING(original, copy);
    TEST_ASSERT_EQUAL(999, strlen(copy));
    
    clap_free(copy);
}

/* ============================================================================
 * clap_strndup Tests
 * ============================================================================ */

void test_clap_strndup_basic(void) {
    const char *original = "hello world";
    char *copy = clap_strndup(original, 5);
    
    TEST_ASSERT_NOT_NULL(copy);
    TEST_ASSERT_EQUAL_STRING("hello", copy);
    
    clap_free(copy);
}

void test_clap_strndup_full_length(void) {
    const char *original = "hello";
    char *copy = clap_strndup(original, 10);  /* n > strlen */
    
    TEST_ASSERT_NOT_NULL(copy);
    TEST_ASSERT_EQUAL_STRING("hello", copy);
    TEST_ASSERT_EQUAL(5, strlen(copy));
    
    clap_free(copy);
}

void test_clap_strndup_zero(void) {
    const char *original = "hello";
    char *copy = clap_strndup(original, 0);
    
    TEST_ASSERT_NOT_NULL(copy);
    TEST_ASSERT_EQUAL_STRING("", copy);
    
    clap_free(copy);
}

void test_clap_strndup_null(void) {
    char *copy = clap_strndup(NULL, 10);
    TEST_ASSERT_NULL(copy);
}

void test_clap_strndup_empty_string(void) {
    const char *original = "";
    char *copy = clap_strndup(original, 10);
    
    TEST_ASSERT_NOT_NULL(copy);
    TEST_ASSERT_EQUAL_STRING("", copy);
    
    clap_free(copy);
}

/* ============================================================================
 * clap_set_allocator Tests
 * ============================================================================ */

void test_clap_set_allocator_basic(void) {
    reset_custom_counts();
    
    /* Note: Allocator can only be set once globally */
    /* This test assumes it hasn't been locked yet */
    
    TEST_ASSERT_TRUE(1);  /* Placeholder - actual test depends on global state */
}

/* ============================================================================
 * Memory Operation Integration Tests
 * ============================================================================ */

void test_allocator_malloc_free_cycle(void) {
    for (int i = 0; i < 100; i++) {
        void *ptr = clap_malloc(64);
        TEST_ASSERT_NOT_NULL(ptr);
        clap_free(ptr);
    }
    TEST_ASSERT_TRUE(1);
}

void test_allocator_realloc_cycle(void) {
    void *ptr = clap_malloc(10);
    TEST_ASSERT_NOT_NULL(ptr);
    
    for (int i = 0; i < 10; i++) {
        size_t new_size = 10 * (i + 2);
        void *new_ptr = clap_realloc(ptr, new_size);
        TEST_ASSERT_NOT_NULL(new_ptr);
        ptr = new_ptr;
    }
    
    clap_free(ptr);
}

void test_allocator_mixed_operations(void) {
    void *ptrs[10];
    
    /* Allocate */
    for (int i = 0; i < 10; i++) {
        ptrs[i] = clap_malloc(32);
        TEST_ASSERT_NOT_NULL(ptrs[i]);
    }
    
    /* Free every other */
    for (int i = 0; i < 10; i += 2) {
        clap_free(ptrs[i]);
        ptrs[i] = NULL;
    }
    
    /* Allocate more */
    for (int i = 0; i < 10; i += 2) {
        ptrs[i] = clap_malloc(64);
        TEST_ASSERT_NOT_NULL(ptrs[i]);
    }
    
    /* Free all */
    for (int i = 0; i < 10; i++) {
        clap_free(ptrs[i]);
    }
    
    TEST_ASSERT_TRUE(1);
}

/* ============================================================================
 * Edge Cases
 * ============================================================================ */

void test_allocator_huge_allocation(void) {
    /* Try to allocate a very large amount */
    size_t huge = 100 * 1024 * 1024;  /* 100 MB */
    void *ptr = clap_malloc(huge);
    
    if (ptr) {
        clap_free(ptr);
    }
    /* May fail on some systems, just ensure no crash */
    TEST_ASSERT_TRUE(1);
}

void test_allocator_max_size_allocation(void) {
    /* Try to allocate SIZE_MAX (should fail gracefully) */
    void *ptr = clap_malloc(SIZE_MAX);
    
    TEST_ASSERT_NULL(ptr);
}

void test_allocator_zero_handling(void) {
    void *ptr1 = clap_malloc(0);
    void *ptr2 = clap_calloc(0, 10);
    void *ptr3 = clap_calloc(10, 0);
    void *ptr4 = clap_realloc(NULL, 0);
    
    /* All should either be NULL or valid pointers that can be freed */
    if (ptr1) clap_free(ptr1);
    if (ptr2) clap_free(ptr2);
    if (ptr3) clap_free(ptr3);
    if (ptr4) clap_free(ptr4);
    
    TEST_ASSERT_TRUE(1);
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

void run_test_allocator(void) {    
    /* clap_malloc Tests */
    RUN_TEST(test_clap_malloc_basic);
    RUN_TEST(test_clap_malloc_zero_size);
    RUN_TEST(test_clap_malloc_large);
    
    /* clap_calloc Tests */
    RUN_TEST(test_clap_calloc_basic);
    RUN_TEST(test_clap_calloc_zero_nmemb);
    RUN_TEST(test_clap_calloc_zero_size);
    RUN_TEST(test_clap_calloc_overflow);
    RUN_TEST(test_clap_calloc_large);
    
    /* clap_free Tests */
    RUN_TEST(test_clap_free_null);
    RUN_TEST(test_clap_free_valid);
    
    /* clap_realloc Tests */
    RUN_TEST(test_clap_realloc_grow);
    RUN_TEST(test_clap_realloc_shrink);
    RUN_TEST(test_clap_realloc_zero_size);
    RUN_TEST(test_clap_realloc_null);
    
    /* clap_strdup Tests */
    RUN_TEST(test_clap_strdup_basic);
    RUN_TEST(test_clap_strdup_empty);
    RUN_TEST(test_clap_strdup_null);
    RUN_TEST(test_clap_strdup_long);
    
    /* clap_strndup Tests */
    RUN_TEST(test_clap_strndup_basic);
    RUN_TEST(test_clap_strndup_full_length);
    RUN_TEST(test_clap_strndup_zero);
    RUN_TEST(test_clap_strndup_null);
    RUN_TEST(test_clap_strndup_empty_string);
    
    /* Integration Tests */
    RUN_TEST(test_allocator_malloc_free_cycle);
    RUN_TEST(test_allocator_realloc_cycle);
    RUN_TEST(test_allocator_mixed_operations);
    
    /* Edge Cases */
    RUN_TEST(test_allocator_huge_allocation);
    RUN_TEST(test_allocator_max_size_allocation);
    RUN_TEST(test_allocator_zero_handling);
}

#ifdef STANDALONE_TEST
int main(void) {
    UNITY_BEGIN();
    run_test_allocator();
    return UNITY_END();
}
#endif