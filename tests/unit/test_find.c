/**
 * @file test_find.c
 * @brief Unit tests for clap_find.c
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

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

static clap_parser_t* create_parser_with_options(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    return parser;
}

/* ============================================================================
 * clap_find_option Tests - Basic
 * ============================================================================ */

void test_find_option_null_parser(void) {
    clap_argument_t *result = clap_find_option(NULL, "verbose", true);
    TEST_ASSERT_NULL(result);
}

void test_find_option_null_name(void) {
    clap_parser_t *parser = create_parser_with_options();

    clap_argument_t *result = clap_find_option(parser, NULL, true);
    TEST_ASSERT_NULL(result);

    clap_parser_free(parser);
}

void test_find_option_empty_name(void) {
    clap_parser_t *parser = create_parser_with_options();
    clap_add_argument(parser, "--verbose");

    clap_argument_t *result = clap_find_option(parser, "", true);
    TEST_ASSERT_NULL(result);

    clap_parser_free(parser);
}

/* ============================================================================
 * clap_find_option Tests - Long Options
 * ============================================================================ */

void test_find_long_option_single(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *expected = clap_add_argument(parser, "--verbose");

    clap_argument_t *result = clap_find_option(parser, "verbose", true);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_PTR(expected, result);

    clap_parser_free(parser);
}

void test_find_long_option_not_found(void) {
    clap_parser_t *parser = create_parser_with_options();
    clap_add_argument(parser, "--verbose");
    clap_add_argument(parser, "--output");

    clap_argument_t *result = clap_find_option(parser, "version", true);

    TEST_ASSERT_NULL(result);

    clap_parser_free(parser);
}

void test_find_long_option_multiple(void) {
    clap_parser_t *parser = create_parser_with_options();
    clap_argument_t *verbose = clap_add_argument(parser, "--verbose");
    clap_argument_t *version = clap_add_argument(parser, "--version");
    clap_argument_t *output = clap_add_argument(parser, "--output");

    TEST_ASSERT_EQUAL_PTR(verbose, clap_find_option(parser, "verbose", true));
    TEST_ASSERT_EQUAL_PTR(version, clap_find_option(parser, "version", true));
    TEST_ASSERT_EQUAL_PTR(output, clap_find_option(parser, "output", true));

    clap_parser_free(parser);
}

void test_find_long_option_case_sensitive(void) {
    clap_parser_t *parser = create_parser_with_options();
    clap_add_argument(parser, "--Verbose");

    /* Should be case-sensitive */
    clap_argument_t *result = clap_find_option(parser, "verbose", true);
    TEST_ASSERT_NULL(result);

    result = clap_find_option(parser, "Verbose", true);
    TEST_ASSERT_NOT_NULL(result);

    clap_parser_free(parser);
}

/* ============================================================================
 * clap_find_option Tests - Short Options
 * ============================================================================ */

void test_find_short_option_single(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *expected = clap_add_argument(parser, "-v");

    clap_argument_t *result = clap_find_option(parser, "v", false);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_PTR(expected, result);

    clap_parser_free(parser);
}

void test_find_short_option_not_found(void) {
    clap_parser_t *parser = create_parser_with_options();
    clap_add_argument(parser, "-v");
    clap_add_argument(parser, "-o");

    clap_argument_t *result = clap_find_option(parser, "x", false);

    TEST_ASSERT_NULL(result);

    clap_parser_free(parser);
}

void test_find_short_option_multiple(void) {
    clap_parser_t *parser = create_parser_with_options();
    clap_argument_t *v = clap_add_argument(parser, "-v");
    clap_argument_t *o = clap_add_argument(parser, "-o");
    clap_argument_t *c = clap_add_argument(parser, "-c");

    TEST_ASSERT_EQUAL_PTR(v, clap_find_option(parser, "v", false));
    TEST_ASSERT_EQUAL_PTR(o, clap_find_option(parser, "o", false));
    TEST_ASSERT_EQUAL_PTR(c, clap_find_option(parser, "c", false));

    clap_parser_free(parser);
}

void test_find_short_option_case_sensitive(void) {
    clap_parser_t *parser = create_parser_with_options();
    clap_add_argument(parser, "-V");

    /* Should be case-sensitive */
    clap_argument_t *result = clap_find_option(parser, "v", false);
    TEST_ASSERT_NULL(result);

    result = clap_find_option(parser, "V", false);
    TEST_ASSERT_NOT_NULL(result);

    clap_parser_free(parser);
}

/* ============================================================================
 * clap_find_option Tests - Combined Options
 * ============================================================================ */

void test_find_option_with_both_forms(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *expected = clap_add_argument(parser, "--verbose/-v");

    clap_argument_t *result1 = clap_find_option(parser, "verbose", true);
    TEST_ASSERT_EQUAL_PTR(expected, result1);

    clap_argument_t *result2 = clap_find_option(parser, "v", false);
    TEST_ASSERT_EQUAL_PTR(expected, result2);

    clap_parser_free(parser);
}

void test_find_option_with_multiple_combined(void) {
    clap_parser_t *parser = create_parser_with_options();
    clap_argument_t *verbose = clap_add_argument(parser, "--verbose/-v");
    clap_argument_t *version = clap_add_argument(parser, "--version/-V");
    clap_argument_t *output = clap_add_argument(parser, "--output/-o");

    TEST_ASSERT_EQUAL_PTR(verbose, clap_find_option(parser, "verbose", true));
    TEST_ASSERT_EQUAL_PTR(verbose, clap_find_option(parser, "v", false));

    TEST_ASSERT_EQUAL_PTR(version, clap_find_option(parser, "version", true));
    TEST_ASSERT_EQUAL_PTR(version, clap_find_option(parser, "V", false));

    TEST_ASSERT_EQUAL_PTR(output, clap_find_option(parser, "output", true));
    TEST_ASSERT_EQUAL_PTR(output, clap_find_option(parser, "o", false));

    clap_parser_free(parser);
}

/* ============================================================================
 * clap_find_option Tests - Edge Cases
 * ============================================================================ */

void test_find_option_with_dashes_in_name(void) {
    clap_parser_t *parser = create_parser_with_options();
    clap_argument_t *expected = clap_add_argument(parser, "--dry-run");

    clap_argument_t *result = clap_find_option(parser, "dry-run", true);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_PTR(expected, result);

    clap_parser_free(parser);
}

void test_find_option_very_long_name(void) {
    clap_parser_t *parser = create_parser_with_options();
    const char *long_name = "--this-is-a-very-long-option-name-that-exceeds-normal-length";
    clap_argument_t *expected = clap_add_argument(parser, long_name);

    /* Extract name without leading dashes */
    clap_argument_t *result = clap_find_option(parser, long_name + 2, true);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_PTR(expected, result);

    clap_parser_free(parser);
}

void test_find_option_with_numbers(void) {
    clap_parser_t *parser = create_parser_with_options();
    clap_argument_t *expected = clap_add_argument(parser, "--port8080");

    clap_argument_t *result = clap_find_option(parser, "port8080", true);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_PTR(expected, result);

    clap_parser_free(parser);
}

void test_find_option_first_match(void) {
    /* If multiple arguments have the same option string (shouldn't happen),
     * the first one should be returned */
    clap_parser_t *parser = create_parser_with_options();
    clap_argument_t *first = clap_add_argument(parser, "--verbose");

    clap_argument_t *result = clap_find_option(parser, "verbose", true);

    TEST_ASSERT_EQUAL_PTR(first, result);

    clap_parser_free(parser);
}

void test_find_option_no_optional_args(void) {
    clap_parser_t *parser = create_parser_with_options();
    /* Only positional arguments */
    clap_add_argument(parser, "input");
    clap_add_argument(parser, "output");

    clap_argument_t *result = clap_find_option(parser, "verbose", true);
    TEST_ASSERT_NULL(result);

    clap_parser_free(parser);
}

/* ============================================================================
 * clap_find_option_fast Tests
 * ============================================================================ */

void test_find_option_fast_basic(void) {
    clap_parser_t *parser = create_parser_with_options();
    clap_argument_t *expected = clap_add_argument(parser, "--verbose");

    clap_argument_t *result = clap_find_option_fast(parser, "verbose", true);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_PTR(expected, result);

    clap_parser_free(parser);
}

void test_find_option_fast_short_option(void) {
    clap_parser_t *parser = create_parser_with_options();
    clap_argument_t *expected = clap_add_argument(parser, "-v");

    clap_argument_t *result = clap_find_option_fast(parser, "v", false);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_PTR(expected, result);

    clap_parser_free(parser);
}

void test_find_option_fast_null_parser(void) {
    clap_argument_t *result = clap_find_option_fast(NULL, "verbose", true);
    TEST_ASSERT_NULL(result);
}

void test_find_option_fast_null_name(void) {
    clap_parser_t *parser = create_parser_with_options();

    clap_argument_t *result = clap_find_option_fast(parser, NULL, true);
    TEST_ASSERT_NULL(result);

    clap_parser_free(parser);
}

void test_find_option_fast_returns_same_as_slow(void) {
    clap_parser_t *parser = create_parser_with_options();
    clap_add_argument(parser, "--verbose/-v");
    clap_add_argument(parser, "--version/-V");
    clap_add_argument(parser, "--output/-o");

    /* Fast and slow should return the same results */
    const char *test_names[] = {"verbose", "v", "version", "V", "output", "o", "nonexistent"};
    bool is_long[] = {true, false, true, false, true, false, true};

    for (int j = 0; j < 7; j++) {
        clap_argument_t *slow = clap_find_option(parser, test_names[j], is_long[j]);
        clap_argument_t *fast = clap_find_option_fast(parser, test_names[j], is_long[j]);
        TEST_ASSERT_EQUAL_PTR(slow, fast);
    }

    clap_parser_free(parser);
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

void run_test_find(void) {
    /* Basic Tests */
    RUN_TEST(test_find_option_null_parser);
    RUN_TEST(test_find_option_null_name);
    RUN_TEST(test_find_option_empty_name);

    /* Long Option Tests */
    RUN_TEST(test_find_long_option_single);
    RUN_TEST(test_find_long_option_not_found);
    RUN_TEST(test_find_long_option_multiple);
    RUN_TEST(test_find_long_option_case_sensitive);

    /* Short Option Tests */
    RUN_TEST(test_find_short_option_single);
    RUN_TEST(test_find_short_option_not_found);
    RUN_TEST(test_find_short_option_multiple);
    RUN_TEST(test_find_short_option_case_sensitive);

    /* Combined Option Tests */
    RUN_TEST(test_find_option_with_both_forms);
    RUN_TEST(test_find_option_with_multiple_combined);

    /* Edge Cases */
    RUN_TEST(test_find_option_with_dashes_in_name);
    RUN_TEST(test_find_option_very_long_name);
    RUN_TEST(test_find_option_with_numbers);
    RUN_TEST(test_find_option_first_match);
    RUN_TEST(test_find_option_no_optional_args);

    /* Fast Option Tests */
    RUN_TEST(test_find_option_fast_basic);
    RUN_TEST(test_find_option_fast_short_option);
    RUN_TEST(test_find_option_fast_null_parser);
    RUN_TEST(test_find_option_fast_null_name);
    RUN_TEST(test_find_option_fast_returns_same_as_slow);
}

#ifdef STANDALONE_TEST
int main(void) {
    UNITY_BEGIN();
    run_test_find();
    return UNITY_END();
}
#endif