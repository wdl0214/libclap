/**
 * @file test_arena.c
 * @brief Unit tests for clap_arena.c
 */

#include "unity.h"
#include "clap_parser_internal.h"
#include <string.h>

/* ============================================================================
 * Test Setup and Teardown
 * ============================================================================ */

void setUp(void) {
    /* Nothing needed */
}

void tearDown(void) {
    /* Nothing needed */
}

static size_t aligned_size(size_t size) {
    return (size + sizeof(void*) - 1) & ~(sizeof(void*) - 1);
}

/* ============================================================================
 * clap_arena_new Tests
 * ============================================================================ */

void test_arena_new_default_chunk_size(void) {
    clap_arena_t *arena = clap_arena_new(0);
    
    TEST_ASSERT_NOT_NULL(arena);
    TEST_ASSERT_EQUAL(4096, arena->chunk_size);
    TEST_ASSERT_NOT_NULL(arena->first);
    TEST_ASSERT_NOT_NULL(arena->current);
    TEST_ASSERT_EQUAL_PTR(arena->first, arena->current);
    TEST_ASSERT_EQUAL(4096, arena->first->capacity);
    TEST_ASSERT_EQUAL(0, arena->first->used);
    TEST_ASSERT_NULL(arena->first->next);
    
    clap_arena_free(arena);
}

void test_arena_new_custom_chunk_size(void) {
    clap_arena_t *arena = clap_arena_new(8192);
    
    TEST_ASSERT_NOT_NULL(arena);
    TEST_ASSERT_EQUAL(8192, arena->chunk_size);
    TEST_ASSERT_EQUAL(8192, arena->first->capacity);
    
    clap_arena_free(arena);
}

void test_arena_new_minimum_chunk_size(void) {
    /* Should enforce minimum of 4096 */
    clap_arena_t *arena = clap_arena_new(1024);
    
    TEST_ASSERT_NOT_NULL(arena);
    TEST_ASSERT_EQUAL(4096, arena->chunk_size);
    
    clap_arena_free(arena);
}

/* ============================================================================
 * clap_arena_free Tests
 * ============================================================================ */

void test_arena_free_null(void) {
    clap_arena_free(NULL);
    TEST_ASSERT_TRUE(1);  /* Should not crash */
}

void test_arena_free_single_chunk(void) {
    clap_arena_t *arena = clap_arena_new(4096);
    
    clap_arena_free(arena);
    TEST_ASSERT_TRUE(1);  /* Should not crash, no memory leaks */
}

void test_arena_free_multiple_chunks(void) {
    clap_arena_t *arena = clap_arena_new(4096);
    
    /* Allocate enough to create multiple chunks */
    for (int i = 0; i < 10; i++) {
        clap_arena_alloc(arena, 1024);
    }
    
    clap_arena_free(arena);
    TEST_ASSERT_TRUE(1);
}

/* ============================================================================
 * clap_arena_alloc Tests
 * ============================================================================ */

void test_arena_alloc_basic(void) {
    clap_arena_t *arena = clap_arena_new(4096);
    
    void *ptr1 = clap_arena_alloc(arena, 100);
    void *ptr2 = clap_arena_alloc(arena, 200);
    
    TEST_ASSERT_NOT_NULL(ptr1);
    TEST_ASSERT_NOT_NULL(ptr2);
    TEST_ASSERT_NOT_EQUAL(ptr1, ptr2);

    size_t expected = aligned_size(100) + aligned_size(200);
    TEST_ASSERT_EQUAL(expected, arena->first->used);
    
    /* Memory should be zero-initialized */
    for (int i = 0; i < 100; i++) {
        TEST_ASSERT_EQUAL(0, ((char*)ptr1)[i]);
    }
    
    clap_arena_free(arena);
}

void test_arena_alloc_zero_size(void) {
    clap_arena_t *arena = clap_arena_new(4096);
    
    void *ptr = clap_arena_alloc(arena, 0);
    TEST_ASSERT_NULL(ptr);
    
    clap_arena_free(arena);
}

void test_arena_alloc_null_arena(void) {
    void *ptr = clap_arena_alloc(NULL, 100);
    TEST_ASSERT_NULL(ptr);
}

void test_arena_alloc_alignment(void) {
    clap_arena_t *arena = clap_arena_new(4096);
    
    void *ptr1 = clap_arena_alloc(arena, 1);
    void *ptr2 = clap_arena_alloc(arena, 1);
    
    /* Should be properly aligned */
    TEST_ASSERT_EQUAL(0, (uintptr_t)ptr1 % sizeof(void*));
    TEST_ASSERT_EQUAL(0, (uintptr_t)ptr2 % sizeof(void*));
    
    clap_arena_free(arena);
}

void test_arena_alloc_creates_new_chunk(void) {
    clap_arena_t *arena = clap_arena_new(4096);
    
    /* First chunk */
    clap_arena_chunk_t *first = arena->first;
    
    /* Allocate more than chunk size to force new chunk */
    void *ptr = clap_arena_alloc(arena, 5000);
    
    TEST_ASSERT_NOT_NULL(ptr);
    TEST_ASSERT_NOT_EQUAL((uintptr_t)first, (uintptr_t)arena->current);
    TEST_ASSERT_EQUAL_PTR(arena->first->next, arena->current);
    TEST_ASSERT_EQUAL(5000, arena->current->capacity);
    
    clap_arena_free(arena);
}

void test_arena_alloc_multiple_chunks(void) {
    clap_arena_t *arena = clap_arena_new(4096);
    
    /* Create several chunks */
    for (int i = 0; i < 5; i++) {
        clap_arena_alloc(arena, 4000);
    }
    
    /* Count chunks */
    int chunk_count = 0;
    clap_arena_chunk_t *chunk = arena->first;
    while (chunk) {
        chunk_count++;
        chunk = chunk->next;
    }
    
    TEST_ASSERT_EQUAL(5, chunk_count);
    
    clap_arena_free(arena);
}

void test_arena_alloc_large_allocation(void) {
    clap_arena_t *arena = clap_arena_new(4096);
    
    /* Allocate larger than default chunk size */
    void *ptr = clap_arena_alloc(arena, 10000);
    
    TEST_ASSERT_NOT_NULL(ptr);
    TEST_ASSERT_EQUAL(10000, arena->current->capacity);
    
    clap_arena_free(arena);
}

/* ============================================================================
 * clap_arena_strdup Tests
 * ============================================================================ */

void test_arena_strdup_basic(void) {
    clap_arena_t *arena = clap_arena_new(4096);
    
    const char *original = "hello world";
    char *copy = clap_arena_strdup(arena, original);

    TEST_ASSERT_NOT_NULL(copy);
    TEST_ASSERT_EQUAL_STRING("hello world", copy);
    TEST_ASSERT_TRUE(original != copy);
    
    clap_arena_free(arena);
}

void test_arena_strdup_empty(void) {
    clap_arena_t *arena = clap_arena_new(4096);
    
    char *copy = clap_arena_strdup(arena, "");
    
    TEST_ASSERT_NOT_NULL(copy);
    TEST_ASSERT_EQUAL_STRING("", copy);
    
    clap_arena_free(arena);
}

void test_arena_strdup_null_arena(void) {
    char *copy = clap_arena_strdup(NULL, "test");
    TEST_ASSERT_NULL(copy);
}

void test_arena_strdup_null_str(void) {
    clap_arena_t *arena = clap_arena_new(4096);
    
    char *copy = clap_arena_strdup(arena, NULL);
    TEST_ASSERT_NULL(copy);
    
    clap_arena_free(arena);
}

void test_arena_strdup_long_string(void) {
    clap_arena_t *arena = clap_arena_new(4096);
    
    const char *long_str = "This is a very long string that will be duplicated";
    char *copy = clap_arena_strdup(arena, long_str);
    
    TEST_ASSERT_NOT_NULL(copy);
    TEST_ASSERT_EQUAL_STRING(long_str, copy);
    
    clap_arena_free(arena);
}

/* ============================================================================
 * clap_arena_reset Tests
 * ============================================================================ */

void test_arena_reset_basic(void) {
    clap_arena_t *arena = clap_arena_new(4096);
    
    /* Allocate some memory */
    void *ptr1 = clap_arena_alloc(arena, 100);
    TEST_ASSERT_EQUAL(aligned_size(100), arena->first->used);
    
    /* Reset */
    clap_arena_reset(arena);
    
    TEST_ASSERT_EQUAL(0, arena->first->used);
    TEST_ASSERT_NULL(arena->first->next);
    TEST_ASSERT_EQUAL_PTR(arena->first, arena->current);
    
    /* Allocate again - should reuse first chunk */
    void *ptr2 = clap_arena_alloc(arena, 100);
    TEST_ASSERT_EQUAL_PTR(ptr1, ptr2);
    TEST_ASSERT_EQUAL(aligned_size(100), arena->first->used);
    
    clap_arena_free(arena);
}

void test_arena_reset_with_multiple_chunks(void) {
    clap_arena_t *arena = clap_arena_new(4096);
    
    /* Create multiple chunks */
    clap_arena_chunk_t *first = arena->first;
    clap_arena_alloc(arena, 5000);
    clap_arena_alloc(arena, 5000);
    
    clap_arena_chunk_t *second = first->next;
    TEST_ASSERT_NOT_NULL(second);
    TEST_ASSERT_NOT_NULL(second->next);
    
    /* Reset */
    clap_arena_reset(arena);
    
    /* Only first chunk should remain */
    TEST_ASSERT_EQUAL_PTR(first, arena->first);
    TEST_ASSERT_EQUAL_PTR(first, arena->current);
    TEST_ASSERT_NULL(first->next);
    TEST_ASSERT_EQUAL(0, first->used);
    
    clap_arena_free(arena);
}

void test_arena_reset_null(void) {
    clap_arena_reset(NULL);
    TEST_ASSERT_TRUE(1);  /* Should not crash */
}

void test_arena_reset_then_allocate(void) {
    clap_arena_t *arena = clap_arena_new(4096);
    
    /* First allocation */
    char *str1 = clap_arena_strdup(arena, "first");
    TEST_ASSERT_EQUAL_STRING("first", str1);
    
    /* Reset */
    clap_arena_reset(arena);
    
    /* Second allocation - should overwrite previous */
    char *str2 = clap_arena_strdup(arena, "second");
    TEST_ASSERT_EQUAL_STRING("second", str2);
    TEST_ASSERT_EQUAL_PTR(str1, str2);  /* Same memory location */
    
    clap_arena_free(arena);
}

/* ============================================================================
 * Arena Usage Pattern Tests
 * ============================================================================ */

void test_arena_mixed_allocations(void) {
    clap_arena_t *arena = clap_arena_new(4096);
    
    /* Mix of different allocation types */
    int *int_ptr = clap_arena_alloc(arena, sizeof(int));
    *int_ptr = 42;
    
    char *str = clap_arena_strdup(arena, "test string");
    
    double *double_ptr = clap_arena_alloc(arena, sizeof(double));
    *double_ptr = 3.14159;
    
    TEST_ASSERT_EQUAL(42, *int_ptr);
    TEST_ASSERT_EQUAL_STRING("test string", str);
    TEST_ASSERT_TRUE(*double_ptr > 3.14 && *double_ptr < 3.15);
    
    clap_arena_free(arena);
}

void test_arena_many_small_allocations(void) {
    clap_arena_t *arena = clap_arena_new(4096);
    
    /* Many small allocations */
    for (int i = 0; i < 1000; i++) {
        void *ptr = clap_arena_alloc(arena, 10);
        TEST_ASSERT_NOT_NULL(ptr);
    }
    
    /* Should have created at least one chunk */
    TEST_ASSERT_NOT_NULL(arena->first);
    
    clap_arena_free(arena);
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

void run_test_arena(void) {    
    /* clap_arena_new Tests */
    RUN_TEST(test_arena_new_default_chunk_size);
    RUN_TEST(test_arena_new_custom_chunk_size);
    RUN_TEST(test_arena_new_minimum_chunk_size);
    
    /* clap_arena_free Tests */
    RUN_TEST(test_arena_free_null);
    RUN_TEST(test_arena_free_single_chunk);
    RUN_TEST(test_arena_free_multiple_chunks);
    
    /* clap_arena_alloc Tests */
    RUN_TEST(test_arena_alloc_basic);
    RUN_TEST(test_arena_alloc_zero_size);
    RUN_TEST(test_arena_alloc_null_arena);
    RUN_TEST(test_arena_alloc_alignment);
    RUN_TEST(test_arena_alloc_creates_new_chunk);
    RUN_TEST(test_arena_alloc_multiple_chunks);
    RUN_TEST(test_arena_alloc_large_allocation);
    
    /* clap_arena_strdup Tests */
    RUN_TEST(test_arena_strdup_basic);
    RUN_TEST(test_arena_strdup_empty);
    RUN_TEST(test_arena_strdup_null_arena);
    RUN_TEST(test_arena_strdup_null_str);
    RUN_TEST(test_arena_strdup_long_string);
    
    /* clap_arena_reset Tests */
    RUN_TEST(test_arena_reset_basic);
    RUN_TEST(test_arena_reset_with_multiple_chunks);
    RUN_TEST(test_arena_reset_null);
    RUN_TEST(test_arena_reset_then_allocate);
    
    /* Usage Pattern Tests */
    RUN_TEST(test_arena_mixed_allocations);
    RUN_TEST(test_arena_many_small_allocations);
}

#ifdef STANDALONE_TEST
int main(void) {
    UNITY_BEGIN();
    run_test_arena();
    return UNITY_END();
}
#endif