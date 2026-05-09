/**
 * @file test_pattern.c
 * @brief Unit tests for clap_pattern.c and clap_parser_stage.c
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
 * looks_like_negative_number Tests
 * ============================================================================ */

void test_looks_like_negative_number_null(void) {
    TEST_ASSERT_FALSE(looks_like_negative_number(NULL));
}

void test_looks_like_negative_number_bare_dash(void) {
    TEST_ASSERT_FALSE(looks_like_negative_number("-"));
}

void test_looks_like_negative_number_empty_string(void) {
    TEST_ASSERT_FALSE(looks_like_negative_number(""));
}

void test_looks_like_negative_number_negative_one(void) {
    TEST_ASSERT_TRUE(looks_like_negative_number("-1"));
}

void test_looks_like_negative_number_negative_zero(void) {
    TEST_ASSERT_TRUE(looks_like_negative_number("-0"));
}

void test_looks_like_negative_number_negative_float(void) {
    TEST_ASSERT_TRUE(looks_like_negative_number("-3.14"));
}

void test_looks_like_negative_number_negative_scientific(void) {
    TEST_ASSERT_TRUE(looks_like_negative_number("-1.5e10"));
}

void test_looks_like_negative_number_positive_number(void) {
    TEST_ASSERT_FALSE(looks_like_negative_number("123"));
}

void test_looks_like_negative_number_positive_float(void) {
    TEST_ASSERT_FALSE(looks_like_negative_number("3.14"));
}

void test_looks_like_negative_number_zero(void) {
    TEST_ASSERT_FALSE(looks_like_negative_number("0"));
}

void test_looks_like_negative_number_non_numeric(void) {
    TEST_ASSERT_FALSE(looks_like_negative_number("abc"));
}

void test_looks_like_negative_number_double_dash_x(void) {
    TEST_ASSERT_FALSE(looks_like_negative_number("--x"));
}

void test_looks_like_negative_number_dash_dash(void) {
    TEST_ASSERT_FALSE(looks_like_negative_number("--"));
}

void test_looks_like_negative_number_negative_one_point_zero(void) {
    TEST_ASSERT_TRUE(looks_like_negative_number("-1.0"));
}

void test_looks_like_negative_number_negative_max_int(void) {
    TEST_ASSERT_TRUE(looks_like_negative_number("-2147483648"));
}

/* ============================================================================
 * clap_analyze_pattern Tests
 * ============================================================================ */

void test_analyze_pattern_null_parser(void) {
    clap_error_t error = {0};
    clap_pattern_t *result = clap_analyze_pattern(NULL, NULL, 0, &error);
    TEST_ASSERT_NULL(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_INVALID_ARGUMENT, error.code);
}

void test_analyze_pattern_tokens_null_with_count(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_error_t error = {0};
    clap_pattern_t *result = clap_analyze_pattern(parser, NULL, 5, &error);
    TEST_ASSERT_NULL(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_INVALID_ARGUMENT, error.code);
    clap_parser_free(parser);
}

void test_analyze_pattern_zero_tokens(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_error_t error = {0};
    clap_pattern_t *result = clap_analyze_pattern(parser, NULL, 0, &error);
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL(0, result->pattern_len);
    TEST_ASSERT_EQUAL(0, result->pattern[0]);
    clap_pattern_free(result);
    clap_parser_free(parser);
}

void test_analyze_pattern_positional_only(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_add_argument(parser, "input");

    char *argv[] = {"prog", "file1.txt"};
    size_t token_count = 0;
    clap_error_t error = {0};
    clap_token_t *tokens = clap_tokenize(2, argv, &token_count, &error);
    TEST_ASSERT_NOT_NULL(tokens);
    TEST_ASSERT_EQUAL(1, token_count);

    clap_pattern_t *pattern = clap_analyze_pattern(parser, tokens, token_count, &error);
    TEST_ASSERT_NOT_NULL(pattern);
    TEST_ASSERT_EQUAL(1, pattern->pattern_len);
    TEST_ASSERT_EQUAL(PATTERN_ARGUMENT, pattern->pattern[0]);
    TEST_ASSERT_EQUAL(0, pattern->option_count);

    clap_pattern_free(pattern);
    clap_tokenize_free(tokens, token_count);
    clap_parser_free(parser);
}

void test_analyze_pattern_option_only(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_add_argument(parser, "--verbose");

    char *argv[] = {"prog", "--verbose"};
    size_t token_count = 0;
    clap_error_t error = {0};
    clap_token_t *tokens = clap_tokenize(2, argv, &token_count, &error);
    TEST_ASSERT_NOT_NULL(tokens);
    TEST_ASSERT_EQUAL(1, token_count);

    clap_pattern_t *pattern = clap_analyze_pattern(parser, tokens, token_count, &error);
    TEST_ASSERT_NOT_NULL(pattern);
    TEST_ASSERT_EQUAL(1, pattern->pattern_len);
    TEST_ASSERT_EQUAL(PATTERN_OPTION, pattern->pattern[0]);
    TEST_ASSERT_EQUAL(1, pattern->option_count);
    TEST_ASSERT_EQUAL(0, pattern->option_indices[0]);

    clap_pattern_free(pattern);
    clap_tokenize_free(tokens, token_count);
    clap_parser_free(parser);
}

void test_analyze_pattern_mixed_options_and_positionals(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_add_argument(parser, "--output");
    clap_add_argument(parser, "--verbose");
    clap_add_argument(parser, "input");

    char *argv[] = {"prog", "--output", "out.txt", "--verbose", "file.txt"};
    size_t token_count = 0;
    clap_error_t error = {0};
    clap_token_t *tokens = clap_tokenize(5, argv, &token_count, &error);
    TEST_ASSERT_NOT_NULL(tokens);
    TEST_ASSERT_EQUAL(4, token_count);

    clap_pattern_t *pattern = clap_analyze_pattern(parser, tokens, token_count, &error);
    TEST_ASSERT_NOT_NULL(pattern);
    TEST_ASSERT_EQUAL(4, pattern->pattern_len);
    /* --output is an option, out.txt is its value (positional if not consumed yet,
       but the pattern analyzer treats the positional token as PATTERN_ARGUMENT and
       stops option parsing) */
    TEST_ASSERT_EQUAL(PATTERN_OPTION, pattern->pattern[0]);   /* --output */
    /* out.txt is a positional token → PATTERN_ARGUMENT, parsing_options becomes false */
    TEST_ASSERT_EQUAL(PATTERN_ARGUMENT, pattern->pattern[1]);  /* out.txt */

    clap_pattern_free(pattern);
    clap_tokenize_free(tokens, token_count);
    clap_parser_free(parser);
}

void test_analyze_pattern_unrecognized_option(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);

    char *argv[] = {"prog", "--nonexistent"};
    size_t token_count = 0;
    clap_error_t error = {0};
    clap_token_t *tokens = clap_tokenize(2, argv, &token_count, &error);
    TEST_ASSERT_NOT_NULL(tokens);

    clap_pattern_t *pattern = clap_analyze_pattern(parser, tokens, token_count, &error);
    TEST_ASSERT_NULL(pattern);
    TEST_ASSERT_EQUAL(CLAP_ERR_UNRECOGNIZED, error.code);

    clap_tokenize_free(tokens, token_count);
    clap_parser_free(parser);
}

void test_analyze_pattern_stop_parsing(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_add_argument(parser, "--verbose");
    clap_add_argument(parser, "input");

    char *argv[] = {"prog", "--verbose", "--", "--file", "-x"};
    size_t token_count = 0;
    clap_error_t error = {0};
    clap_token_t *tokens = clap_tokenize(5, argv, &token_count, &error);
    TEST_ASSERT_NOT_NULL(tokens);
    TEST_ASSERT_EQUAL(4, token_count);

    clap_pattern_t *pattern = clap_analyze_pattern(parser, tokens, token_count, &error);
    TEST_ASSERT_NOT_NULL(pattern);
    TEST_ASSERT_EQUAL(4, pattern->pattern_len);
    TEST_ASSERT_EQUAL(PATTERN_OPTION, pattern->pattern[0]);   /* --verbose */
    TEST_ASSERT_EQUAL(PATTERN_STOP, pattern->pattern[1]);     /* -- */
    TEST_ASSERT_EQUAL(PATTERN_ARGUMENT, pattern->pattern[2]);  /* --file */
    TEST_ASSERT_EQUAL(PATTERN_ARGUMENT, pattern->pattern[3]);  /* -x */

    clap_pattern_free(pattern);
    clap_tokenize_free(tokens, token_count);
    clap_parser_free(parser);
}

void test_analyze_pattern_negative_number_as_positional(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_add_argument(parser, "--threshold");

    char *argv[] = {"prog", "--threshold", "-5"};
    size_t token_count = 0;
    clap_error_t error = {0};
    clap_token_t *tokens = clap_tokenize(3, argv, &token_count, &error);
    TEST_ASSERT_NOT_NULL(tokens);
    TEST_ASSERT_EQUAL(2, token_count);

    /* -5 is tokenized as TOKEN_SHORT_OPTION ("5") but looks_like_negative_number
       makes the pattern analyzer treat it as PATTERN_ARGUMENT */
    clap_pattern_t *pattern = clap_analyze_pattern(parser, tokens, token_count, &error);
    TEST_ASSERT_NOT_NULL(pattern);
    TEST_ASSERT_EQUAL(PATTERN_OPTION, pattern->pattern[0]);     /* --threshold */
    TEST_ASSERT_EQUAL(PATTERN_ARGUMENT, pattern->pattern[1]);   /* -5 */
    TEST_ASSERT_EQUAL(1, pattern->option_count);

    clap_pattern_free(pattern);
    clap_tokenize_free(tokens, token_count);
    clap_parser_free(parser);
}

void test_analyze_pattern_short_bundle(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_add_argument(parser, "-v");
    clap_add_argument(parser, "-f");

    char *argv[] = {"prog", "-vf"};
    size_t token_count = 0;
    clap_error_t error = {0};
    clap_token_t *tokens = clap_tokenize(2, argv, &token_count, &error);
    TEST_ASSERT_NOT_NULL(tokens);
    TEST_ASSERT_EQUAL(1, token_count);
    TEST_ASSERT_EQUAL(TOKEN_SHORT_OPTION_BUNDLE, tokens[0].type);

    clap_pattern_t *pattern = clap_analyze_pattern(parser, tokens, token_count, &error);
    TEST_ASSERT_NOT_NULL(pattern);
    TEST_ASSERT_EQUAL(1, pattern->pattern_len);
    TEST_ASSERT_EQUAL(PATTERN_OPTION, pattern->pattern[0]);
    TEST_ASSERT_EQUAL(1, pattern->option_count);
    TEST_ASSERT_EQUAL(0, pattern->option_indices[0]);

    clap_pattern_free(pattern);
    clap_tokenize_free(tokens, token_count);
    clap_parser_free(parser);
}

void test_analyze_pattern_long_option_with_equals(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_add_argument(parser, "--output");

    char *argv[] = {"prog", "--output=result.txt"};
    size_t token_count = 0;
    clap_error_t error = {0};
    clap_token_t *tokens = clap_tokenize(2, argv, &token_count, &error);
    TEST_ASSERT_NOT_NULL(tokens);
    TEST_ASSERT_EQUAL(1, token_count);
    TEST_ASSERT_EQUAL(TOKEN_LONG_OPTION_EQ, tokens[0].type);

    clap_pattern_t *pattern = clap_analyze_pattern(parser, tokens, token_count, &error);
    TEST_ASSERT_NOT_NULL(pattern);
    TEST_ASSERT_EQUAL(1, pattern->pattern_len);
    TEST_ASSERT_EQUAL(PATTERN_OPTION, pattern->pattern[0]);
    TEST_ASSERT_EQUAL(1, pattern->option_count);

    clap_pattern_free(pattern);
    clap_tokenize_free(tokens, token_count);
    clap_parser_free(parser);
}

void test_analyze_pattern_ambiguous_option(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_parser_set_allow_abbrev(parser, true);
    clap_add_argument(parser, "--verbose");
    clap_add_argument(parser, "--version");

    char *argv[] = {"prog", "--ver"};
    size_t token_count = 0;
    clap_error_t error = {0};
    clap_token_t *tokens = clap_tokenize(2, argv, &token_count, &error);
    TEST_ASSERT_NOT_NULL(tokens);

    clap_pattern_t *pattern = clap_analyze_pattern(parser, tokens, token_count, &error);
    TEST_ASSERT_NULL(pattern);
    TEST_ASSERT_EQUAL(CLAP_ERR_UNRECOGNIZED, error.code);

    clap_tokenize_free(tokens, token_count);
    clap_parser_free(parser);
}

void test_analyze_pattern_multiple_positionals(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_add_argument(parser, "src");
    clap_add_argument(parser, "dst");

    char *argv[] = {"prog", "a.txt", "b.txt"};
    size_t token_count = 0;
    clap_error_t error = {0};
    clap_token_t *tokens = clap_tokenize(3, argv, &token_count, &error);
    TEST_ASSERT_NOT_NULL(tokens);
    TEST_ASSERT_EQUAL(2, token_count);

    clap_pattern_t *pattern = clap_analyze_pattern(parser, tokens, token_count, &error);
    TEST_ASSERT_NOT_NULL(pattern);
    TEST_ASSERT_EQUAL(2, pattern->pattern_len);
    TEST_ASSERT_EQUAL(PATTERN_ARGUMENT, pattern->pattern[0]);
    TEST_ASSERT_EQUAL(PATTERN_ARGUMENT, pattern->pattern[1]);
    TEST_ASSERT_EQUAL(0, pattern->option_count);

    clap_pattern_free(pattern);
    clap_tokenize_free(tokens, token_count);
    clap_parser_free(parser);
}

/* ============================================================================
 * clap_pattern_free Tests
 * ============================================================================ */

void test_pattern_free_basic(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_add_argument(parser, "--verbose");

    char *argv[] = {"prog", "--verbose"};
    size_t token_count = 0;
    clap_error_t error = {0};
    clap_token_t *tokens = clap_tokenize(2, argv, &token_count, &error);
    clap_pattern_t *pattern = clap_analyze_pattern(parser, tokens, token_count, &error);
    TEST_ASSERT_NOT_NULL(pattern);

    /* Should not crash */
    clap_pattern_free(pattern);

    clap_tokenize_free(tokens, token_count);
    clap_parser_free(parser);
}

void test_pattern_free_null(void) {
    /* Should not crash */
    clap_pattern_free(NULL);
}

/* ============================================================================
 * clap_parse_with_pattern Tests
 * ============================================================================ */

void test_parse_with_pattern_null_params(void) {
    clap_error_t error = {0};
    clap_parse_result_t result = clap_parse_with_pattern(NULL, NULL, NULL, NULL, NULL, &error);
    TEST_ASSERT_EQUAL(CLAP_PARSE_ERROR, result);
}

void test_parse_with_pattern_basic_positional(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_add_argument(parser, "input");

    char *argv[] = {"prog", "file.txt"};
    size_t token_count = 0;
    clap_error_t error = {0};
    clap_token_t *tokens = clap_tokenize(2, argv, &token_count, &error);
    clap_pattern_t *pattern = clap_analyze_pattern(parser, tokens, token_count, &error);
    clap_namespace_t *ns = clap_namespace_new();

    clap_parse_result_t result = clap_parse_with_pattern(parser, tokens, pattern, ns, NULL, &error);
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);

    const char *value = NULL;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "input", &value));
    TEST_ASSERT_EQUAL_STRING("file.txt", value);

    clap_namespace_free(ns);
    clap_pattern_free(pattern);
    clap_tokenize_free(tokens, token_count);
    clap_parser_free(parser);
}

void test_parse_with_pattern_store_true(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);

    clap_argument_t *arg = clap_add_argument(parser, "-v/--verbose");
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);

    char *argv[] = {"prog", "-v"};
    size_t token_count = 0;
    clap_error_t error = {0};
    clap_token_t *tokens = clap_tokenize(2, argv, &token_count, &error);
    clap_pattern_t *pattern = clap_analyze_pattern(parser, tokens, token_count, &error);
    clap_namespace_t *ns = clap_namespace_new();

    clap_parse_result_t result = clap_parse_with_pattern(parser, tokens, pattern, ns, NULL, &error);
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);

    bool value = false;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "verbose", &value));
    TEST_ASSERT_TRUE(value);

    clap_namespace_free(ns);
    clap_pattern_free(pattern);
    clap_tokenize_free(tokens, token_count);
    clap_parser_free(parser);
}

void test_parse_with_pattern_help_action(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--help");
    clap_argument_action(arg, CLAP_ACTION_HELP);

    char *argv[] = {"prog", "--help"};
    size_t token_count = 0;
    clap_error_t error = {0};
    clap_token_t *tokens = clap_tokenize(2, argv, &token_count, &error);
    clap_pattern_t *pattern = clap_analyze_pattern(parser, tokens, token_count, &error);
    clap_namespace_t *ns = clap_namespace_new();

    clap_parse_result_t result = clap_parse_with_pattern(parser, tokens, pattern, ns, NULL, &error);
    TEST_ASSERT_EQUAL(CLAP_PARSE_HELP, result);

    clap_namespace_free(ns);
    clap_pattern_free(pattern);
    clap_tokenize_free(tokens, token_count);
    clap_parser_free(parser);
}

void test_parse_with_pattern_version_action(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_parser_set_version(parser, "1.0.0");
    clap_argument_t *arg = clap_add_argument(parser, "--version");
    clap_argument_action(arg, CLAP_ACTION_VERSION);

    char *argv[] = {"prog", "--version"};
    size_t token_count = 0;
    clap_error_t error = {0};
    clap_token_t *tokens = clap_tokenize(2, argv, &token_count, &error);
    clap_pattern_t *pattern = clap_analyze_pattern(parser, tokens, token_count, &error);
    clap_namespace_t *ns = clap_namespace_new();

    clap_parse_result_t result = clap_parse_with_pattern(parser, tokens, pattern, ns, NULL, &error);
    TEST_ASSERT_EQUAL(CLAP_PARSE_VERSION, result);

    clap_namespace_free(ns);
    clap_pattern_free(pattern);
    clap_tokenize_free(tokens, token_count);
    clap_parser_free(parser);
}

void test_parse_with_pattern_unrecognized_option(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);

    char *argv[] = {"prog", "--bad"};
    size_t token_count = 0;
    clap_error_t error = {0};
    clap_token_t *tokens = clap_tokenize(2, argv, &token_count, &error);

    clap_pattern_t *pattern = clap_analyze_pattern(parser, tokens, token_count, &error);
    /* Pattern analysis catches unrecognized options */
    TEST_ASSERT_NULL(pattern);

    clap_tokenize_free(tokens, token_count);
    clap_parser_free(parser);
}

void test_parse_with_pattern_option_with_value(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_add_argument(parser, "--output");

    char *argv[] = {"prog", "--output", "result.txt"};
    size_t token_count = 0;
    clap_error_t error = {0};
    clap_token_t *tokens = clap_tokenize(3, argv, &token_count, &error);
    clap_pattern_t *pattern = clap_analyze_pattern(parser, tokens, token_count, &error);
    clap_namespace_t *ns = clap_namespace_new();

    clap_parse_result_t result = clap_parse_with_pattern(parser, tokens, pattern, ns, NULL, &error);
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);

    const char *value = NULL;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "output", &value));
    TEST_ASSERT_EQUAL_STRING("result.txt", value);

    clap_namespace_free(ns);
    clap_pattern_free(pattern);
    clap_tokenize_free(tokens, token_count);
    clap_parser_free(parser);
}

void test_parse_with_pattern_count_increment(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "-v/--verbose");
    clap_argument_action(arg, CLAP_ACTION_COUNT);

    char *argv[] = {"prog", "-v", "-v", "-v"};
    size_t token_count = 0;
    clap_error_t error = {0};
    clap_token_t *tokens = clap_tokenize(4, argv, &token_count, &error);
    clap_pattern_t *pattern = clap_analyze_pattern(parser, tokens, token_count, &error);
    clap_namespace_t *ns = clap_namespace_new();

    clap_parse_result_t result = clap_parse_with_pattern(parser, tokens, pattern, ns, NULL, &error);
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);

    int count = -1;
    TEST_ASSERT_TRUE(clap_namespace_get_int(ns, "verbose", &count));
    TEST_ASSERT_EQUAL(3, count);

    clap_namespace_free(ns);
    clap_pattern_free(pattern);
    clap_tokenize_free(tokens, token_count);
    clap_parser_free(parser);
}

void test_parse_with_pattern_too_many_positionals(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_add_argument(parser, "input");

    char *argv[] = {"prog", "a.txt", "b.txt"};
    size_t token_count = 0;
    clap_error_t error = {0};
    clap_token_t *tokens = clap_tokenize(3, argv, &token_count, &error);
    clap_pattern_t *pattern = clap_analyze_pattern(parser, tokens, token_count, &error);
    clap_namespace_t *ns = clap_namespace_new();

    clap_parse_result_t result = clap_parse_with_pattern(parser, tokens, pattern, ns, NULL, &error);
    TEST_ASSERT_EQUAL(CLAP_PARSE_ERROR, result);
    TEST_ASSERT_EQUAL(CLAP_ERR_TOO_MANY_ARGS, error.code);

    clap_namespace_free(ns);
    clap_pattern_free(pattern);
    clap_tokenize_free(tokens, token_count);
    clap_parser_free(parser);
}

void test_parse_with_pattern_short_bundle(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg_v = clap_add_argument(parser, "-v");
    clap_argument_action(arg_v, CLAP_ACTION_STORE_TRUE);

    clap_argument_t *arg_f = clap_add_argument(parser, "-f");
    clap_argument_action(arg_f, CLAP_ACTION_STORE_TRUE);

    char *argv[] = {"prog", "-vf"};
    size_t token_count = 0;
    clap_error_t error = {0};
    clap_token_t *tokens = clap_tokenize(2, argv, &token_count, &error);
    clap_pattern_t *pattern = clap_analyze_pattern(parser, tokens, token_count, &error);
    clap_namespace_t *ns = clap_namespace_new();

    clap_parse_result_t result = clap_parse_with_pattern(parser, tokens, pattern, ns, NULL, &error);
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);

    bool v = false, f = false;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "v", &v));
    TEST_ASSERT_TRUE(v);
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "f", &f));
    TEST_ASSERT_TRUE(f);

    clap_namespace_free(ns);
    clap_pattern_free(pattern);
    clap_tokenize_free(tokens, token_count);
    clap_parser_free(parser);
}

void test_parse_with_pattern_stop_parsing_positional_after_stop(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_add_argument(parser, "input");

    char *argv[] = {"prog", "--", "--file"};
    size_t token_count = 0;
    clap_error_t error = {0};
    clap_token_t *tokens = clap_tokenize(3, argv, &token_count, &error);
    clap_pattern_t *pattern = clap_analyze_pattern(parser, tokens, token_count, &error);
    clap_namespace_t *ns = clap_namespace_new();

    clap_parse_result_t result = clap_parse_with_pattern(parser, tokens, pattern, ns, NULL, &error);
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);

    const char *value = NULL;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "input", &value));
    TEST_ASSERT_EQUAL_STRING("--file", value);

    clap_namespace_free(ns);
    clap_pattern_free(pattern);
    clap_tokenize_free(tokens, token_count);
    clap_parser_free(parser);
}

/* ============================================================================
 * Test Runner
 * ============================================================================ */

void run_test_pattern(void) {
    /* looks_like_negative_number */
    RUN_TEST(test_looks_like_negative_number_null);
    RUN_TEST(test_looks_like_negative_number_bare_dash);
    RUN_TEST(test_looks_like_negative_number_empty_string);
    RUN_TEST(test_looks_like_negative_number_negative_one);
    RUN_TEST(test_looks_like_negative_number_negative_zero);
    RUN_TEST(test_looks_like_negative_number_negative_float);
    RUN_TEST(test_looks_like_negative_number_negative_scientific);
    RUN_TEST(test_looks_like_negative_number_positive_number);
    RUN_TEST(test_looks_like_negative_number_positive_float);
    RUN_TEST(test_looks_like_negative_number_zero);
    RUN_TEST(test_looks_like_negative_number_non_numeric);
    RUN_TEST(test_looks_like_negative_number_double_dash_x);
    RUN_TEST(test_looks_like_negative_number_dash_dash);
    RUN_TEST(test_looks_like_negative_number_negative_one_point_zero);
    RUN_TEST(test_looks_like_negative_number_negative_max_int);

    /* clap_analyze_pattern */
    RUN_TEST(test_analyze_pattern_null_parser);
    RUN_TEST(test_analyze_pattern_tokens_null_with_count);
    RUN_TEST(test_analyze_pattern_zero_tokens);
    RUN_TEST(test_analyze_pattern_positional_only);
    RUN_TEST(test_analyze_pattern_option_only);
    RUN_TEST(test_analyze_pattern_mixed_options_and_positionals);
    RUN_TEST(test_analyze_pattern_unrecognized_option);
    RUN_TEST(test_analyze_pattern_stop_parsing);
    RUN_TEST(test_analyze_pattern_negative_number_as_positional);
    RUN_TEST(test_analyze_pattern_short_bundle);
    RUN_TEST(test_analyze_pattern_long_option_with_equals);
    RUN_TEST(test_analyze_pattern_ambiguous_option);
    RUN_TEST(test_analyze_pattern_multiple_positionals);

    /* clap_pattern_free */
    RUN_TEST(test_pattern_free_basic);
    RUN_TEST(test_pattern_free_null);

    /* clap_parse_with_pattern */
    RUN_TEST(test_parse_with_pattern_null_params);
    RUN_TEST(test_parse_with_pattern_basic_positional);
    RUN_TEST(test_parse_with_pattern_store_true);
    RUN_TEST(test_parse_with_pattern_help_action);
    RUN_TEST(test_parse_with_pattern_version_action);
    RUN_TEST(test_parse_with_pattern_unrecognized_option);
    RUN_TEST(test_parse_with_pattern_option_with_value);
    RUN_TEST(test_parse_with_pattern_count_increment);
    RUN_TEST(test_parse_with_pattern_too_many_positionals);
    RUN_TEST(test_parse_with_pattern_short_bundle);
    RUN_TEST(test_parse_with_pattern_stop_parsing_positional_after_stop);
}

#ifdef STANDALONE_TEST
int main(void) {
    UNITY_BEGIN();
    run_test_pattern();
    return UNITY_END();
}
#endif
