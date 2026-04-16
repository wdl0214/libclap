/**
 * @file test_trie.c
 * @brief Unit tests for clap_trie.c
 */

#include "unity.h"
#include "clap_parser_internal.h"
#include <string.h>

/* ============================================================================
 * Test Setup and Teardown
 * ============================================================================ */

static clap_parser_t *g_parser = NULL;

void setUp(void) {
    g_parser = clap_parser_new("prog", NULL, NULL);
}

void tearDown(void) {
    clap_parser_free(g_parser);
    g_parser = NULL;
}

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

static clap_argument_t* create_arg(const char *name) {
    return clap_add_argument(g_parser, name);
}

/* ============================================================================
 * clap_trie_new Tests
 * ============================================================================ */

void test_trie_new_basic(void) {
    clap_trie_t *trie = clap_trie_new();
    
    TEST_ASSERT_NOT_NULL(trie);
    TEST_ASSERT_NOT_NULL(trie->arena);
    TEST_ASSERT_NOT_NULL(trie->root);
    TEST_ASSERT_EQUAL(0, trie->node_count);
    
    clap_trie_free(trie);
}

/* ============================================================================
 * clap_trie_free Tests
 * ============================================================================ */

void test_trie_free_null(void) {
    clap_trie_free(NULL);
    TEST_ASSERT_TRUE(1);  /* Should not crash */
}

void test_trie_free_with_nodes(void) {
    clap_trie_t *trie = clap_trie_new();
    clap_argument_t *arg = create_arg("--help");
    
    clap_trie_insert(trie, "help", arg);
    
    clap_trie_free(trie);
    TEST_ASSERT_TRUE(1);  /* Should not crash, no memory leaks */
}

/* ============================================================================
 * clap_trie_insert Tests
 * ============================================================================ */

void test_trie_insert_basic(void) {
    clap_trie_t *trie = clap_trie_new();
    clap_argument_t *arg = create_arg("--help");
    
    bool result = clap_trie_insert(trie, "help", arg);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(trie->node_count > 0);
    
    clap_trie_free(trie);
}

void test_trie_insert_multiple(void) {
    clap_trie_t *trie = clap_trie_new();
    clap_argument_t *arg1 = create_arg("--help");
    clap_argument_t *arg2 = create_arg("--version");
    clap_argument_t *arg3 = create_arg("--verbose");
    
    TEST_ASSERT_TRUE(clap_trie_insert(trie, "help", arg1));
    TEST_ASSERT_TRUE(clap_trie_insert(trie, "version", arg2));
    TEST_ASSERT_TRUE(clap_trie_insert(trie, "verbose", arg3));
    
    clap_trie_free(trie);
}

void test_trie_insert_duplicate(void) {
    clap_trie_t *trie = clap_trie_new();
    clap_argument_t *arg1 = create_arg("--help");
    clap_argument_t *arg2 = create_arg("--help");
    
    TEST_ASSERT_TRUE(clap_trie_insert(trie, "help", arg1));
    TEST_ASSERT_FALSE(clap_trie_insert(trie, "help", arg2));  /* Duplicate */
    
    clap_trie_free(trie);
}

void test_trie_insert_null_params(void) {
    clap_trie_t *trie = clap_trie_new();
    clap_argument_t *arg = create_arg("--help");
    
    TEST_ASSERT_FALSE(clap_trie_insert(NULL, "help", arg));
    TEST_ASSERT_FALSE(clap_trie_insert(trie, NULL, arg));
    TEST_ASSERT_FALSE(clap_trie_insert(trie, "help", NULL));
    
    clap_trie_free(trie);
}

void test_trie_insert_empty_key(void) {
    clap_trie_t *trie = clap_trie_new();
    clap_argument_t *arg = create_arg("--help");
    
    /* Empty key - should insert at root? */
    bool result = clap_trie_insert(trie, "", arg);
    
    /* Root is already a node, should set is_end_of_word */
    TEST_ASSERT_TRUE(result);
    
    clap_trie_free(trie);
}

void test_trie_insert_long_key(void) {
    clap_trie_t *trie = clap_trie_new();
    clap_argument_t *arg = create_arg("--long-option");
    const char *long_key = "this-is-a-very-long-option-name";
    
    bool result = clap_trie_insert(trie, long_key, arg);
    
    TEST_ASSERT_TRUE(result);
    
    clap_trie_free(trie);
}

void test_trie_insert_shared_prefix(void) {
    clap_trie_t *trie = clap_trie_new();
    clap_argument_t *arg1 = create_arg("--help");
    clap_argument_t *arg2 = create_arg("--hello");
    
    TEST_ASSERT_TRUE(clap_trie_insert(trie, "help", arg1));
    TEST_ASSERT_TRUE(clap_trie_insert(trie, "hello", arg2));
    
    /* Should share the "hel" prefix nodes */
    
    clap_trie_free(trie);
}

/* ============================================================================
 * clap_trie_find_exact Tests
 * ============================================================================ */

void test_trie_find_exact_basic(void) {
    clap_trie_t *trie = clap_trie_new();
    clap_argument_t *arg = create_arg("--help");
    
    clap_trie_insert(trie, "help", arg);
    
    clap_argument_t *found = clap_trie_find_exact(trie, "help");
    
    TEST_ASSERT_EQUAL_PTR(arg, found);
    
    clap_trie_free(trie);
}

void test_trie_find_exact_not_found(void) {
    clap_trie_t *trie = clap_trie_new();
    clap_argument_t *arg = create_arg("--help");
    
    clap_trie_insert(trie, "help", arg);
    
    clap_argument_t *found = clap_trie_find_exact(trie, "version");
    
    TEST_ASSERT_NULL(found);
    
    clap_trie_free(trie);
}

void test_trie_find_exact_empty_trie(void) {
    clap_trie_t *trie = clap_trie_new();
    
    clap_argument_t *found = clap_trie_find_exact(trie, "help");
    
    TEST_ASSERT_NULL(found);
    
    clap_trie_free(trie);
}

void test_trie_find_exact_null_params(void) {
    clap_trie_t *trie = clap_trie_new();
    clap_argument_t *arg = create_arg("--help");
    clap_trie_insert(trie, "help", arg);
    
    TEST_ASSERT_NULL(clap_trie_find_exact(NULL, "help"));
    TEST_ASSERT_NULL(clap_trie_find_exact(trie, NULL));
    
    clap_trie_free(trie);
}

void test_trie_find_exact_multiple(void) {
    clap_trie_t *trie = clap_trie_new();
    clap_argument_t *arg1 = create_arg("--help");
    clap_argument_t *arg2 = create_arg("--version");
    clap_argument_t *arg3 = create_arg("--verbose");
    
    clap_trie_insert(trie, "help", arg1);
    clap_trie_insert(trie, "version", arg2);
    clap_trie_insert(trie, "verbose", arg3);
    
    TEST_ASSERT_EQUAL_PTR(arg1, clap_trie_find_exact(trie, "help"));
    TEST_ASSERT_EQUAL_PTR(arg2, clap_trie_find_exact(trie, "version"));
    TEST_ASSERT_EQUAL_PTR(arg3, clap_trie_find_exact(trie, "verbose"));
    
    clap_trie_free(trie);
}

void test_trie_find_exact_prefix_not_full_word(void) {
    clap_trie_t *trie = clap_trie_new();
    clap_argument_t *arg = create_arg("--help");
    
    clap_trie_insert(trie, "help", arg);
    
    /* "hel" is a prefix, but not a complete word */
    clap_argument_t *found = clap_trie_find_exact(trie, "hel");
    
    TEST_ASSERT_NULL(found);
    
    clap_trie_free(trie);
}

/* ============================================================================
 * clap_trie_find_prefix Tests
 * ============================================================================ */

void test_trie_find_prefix_exact_match(void) {
    clap_trie_t *trie = clap_trie_new();
    clap_argument_t *arg = create_arg("--help");
    
    clap_trie_insert(trie, "help", arg);
    
    clap_argument_t *found = clap_trie_find_prefix(trie, "help", false);
    
    TEST_ASSERT_EQUAL_PTR(arg, found);
    
    clap_trie_free(trie);
}

void test_trie_find_prefix_partial(void) {
    clap_trie_t *trie = clap_trie_new();
    clap_argument_t *arg = create_arg("--help");
    
    clap_trie_insert(trie, "help", arg);
    
    /* "hel" is prefix of "help" */
    clap_argument_t *found = clap_trie_find_prefix(trie, "hel", false);
    
    TEST_ASSERT_EQUAL_PTR(arg, found);
    
    clap_trie_free(trie);
}

void test_trie_find_prefix_not_found(void) {
    clap_trie_t *trie = clap_trie_new();
    clap_argument_t *arg = create_arg("--help");
    
    clap_trie_insert(trie, "help", arg);
    
    clap_argument_t *found = clap_trie_find_prefix(trie, "xyz", false);
    
    TEST_ASSERT_NULL(found);
    
    clap_trie_free(trie);
}

void test_trie_find_prefix_ambiguous(void) {
    clap_trie_t *trie = clap_trie_new();
    clap_argument_t *arg1 = create_arg("--help");
    clap_argument_t *arg2 = create_arg("--hello");
    
    clap_trie_insert(trie, "help", arg1);
    clap_trie_insert(trie, "hello", arg2);
    
    /* "hel" is prefix of both - ambiguous */
    /* allow_ambiguous = false, should return first found */
    clap_argument_t *found = clap_trie_find_prefix(trie, "hel", false);
    
    /* Should return one of them */
    TEST_ASSERT_NOT_NULL(found);
    TEST_ASSERT(found == arg1 || found == arg2);
    
    clap_trie_free(trie);
}

void test_trie_find_prefix_null_params(void) {
    clap_trie_t *trie = clap_trie_new();
    clap_argument_t *arg = create_arg("--help");
    clap_trie_insert(trie, "help", arg);
    
    TEST_ASSERT_NULL(clap_trie_find_prefix(NULL, "help", false));
    TEST_ASSERT_NULL(clap_trie_find_prefix(trie, NULL, false));
    
    clap_trie_free(trie);
}

void test_trie_find_prefix_single_char(void) {
    clap_trie_t *trie = clap_trie_new();
    clap_argument_t *arg = create_arg("-h");
    
    clap_trie_insert(trie, "h", arg);
    
    clap_argument_t *found = clap_trie_find_prefix(trie, "h", false);
    
    TEST_ASSERT_EQUAL_PTR(arg, found);
    
    clap_trie_free(trie);
}

/* ============================================================================
 * Trie Integration Tests
 * ============================================================================ */

void test_trie_insert_find_cycle(void) {
    clap_trie_t *trie = clap_trie_new();
    
    const char *keys[] = {"help", "version", "verbose", "output", "input"};
    clap_argument_t *args[5];
    
    for (int i = 0; i < 5; i++) {
        args[i] = create_arg("--dummy");
        TEST_ASSERT_TRUE(clap_trie_insert(trie, keys[i], args[i]));
    }
    
    for (int i = 0; i < 5; i++) {
        clap_argument_t *found = clap_trie_find_exact(trie, keys[i]);
        TEST_ASSERT_EQUAL_PTR(args[i], found);
    }
    
    clap_trie_free(trie);
}

void test_trie_case_sensitivity(void) {
    clap_trie_t *trie = clap_trie_new();
    clap_argument_t *arg = create_arg("--Help");
    
    clap_trie_insert(trie, "Help", arg);
    
    /* Should be case-sensitive */
    TEST_ASSERT_NULL(clap_trie_find_exact(trie, "help"));
    TEST_ASSERT_NOT_NULL(clap_trie_find_exact(trie, "Help"));
    
    clap_trie_free(trie);
}

void test_trie_with_special_characters(void) {
    clap_trie_t *trie = clap_trie_new();
    clap_argument_t *arg = create_arg("--with-dash");
    
    clap_trie_insert(trie, "with-dash", arg);
    
    clap_argument_t *found = clap_trie_find_exact(trie, "with-dash");
    TEST_ASSERT_EQUAL_PTR(arg, found);
    
    clap_trie_free(trie);
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

void run_test_trie(void) {
    /* clap_trie_new Tests */
    RUN_TEST(test_trie_new_basic);
    
    /* clap_trie_free Tests */
    RUN_TEST(test_trie_free_null);
    RUN_TEST(test_trie_free_with_nodes);
    
    /* clap_trie_insert Tests */
    RUN_TEST(test_trie_insert_basic);
    RUN_TEST(test_trie_insert_multiple);
    RUN_TEST(test_trie_insert_duplicate);
    RUN_TEST(test_trie_insert_null_params);
    RUN_TEST(test_trie_insert_empty_key);
    RUN_TEST(test_trie_insert_long_key);
    RUN_TEST(test_trie_insert_shared_prefix);
    
    /* clap_trie_find_exact Tests */
    RUN_TEST(test_trie_find_exact_basic);
    RUN_TEST(test_trie_find_exact_not_found);
    RUN_TEST(test_trie_find_exact_empty_trie);
    RUN_TEST(test_trie_find_exact_null_params);
    RUN_TEST(test_trie_find_exact_multiple);
    RUN_TEST(test_trie_find_exact_prefix_not_full_word);
    
    /* clap_trie_find_prefix Tests */
    RUN_TEST(test_trie_find_prefix_exact_match);
    RUN_TEST(test_trie_find_prefix_partial);
    RUN_TEST(test_trie_find_prefix_not_found);
    RUN_TEST(test_trie_find_prefix_ambiguous);
    RUN_TEST(test_trie_find_prefix_null_params);
    RUN_TEST(test_trie_find_prefix_single_char);
    
    /* Integration Tests */
    RUN_TEST(test_trie_insert_find_cycle);
    RUN_TEST(test_trie_case_sensitivity);
    RUN_TEST(test_trie_with_special_characters);
}

#ifdef STANDALONE_TEST
int main(void) {
    UNITY_BEGIN();
    run_test_trie();
    return UNITY_END();
}
#endif