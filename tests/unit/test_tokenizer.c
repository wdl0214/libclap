/**
 * @file test_tokenizer.c
 * @brief Unit tests for clap_tokenizer.c
 */

/* Include source directly to access static functions */
#include "unity.h"
#include "clap_parser_internal.h"

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
 * Tokenize Tests - Basic
 * ============================================================================ */

void test_tokenize_null(void) {
    token_t token = clap_tokenize_arg(NULL);
    TEST_ASSERT_EQUAL(TOKEN_END, token.type);
}

void test_tokenize_positional(void) {
    token_t token = clap_tokenize_arg("input.txt");
    TEST_ASSERT_EQUAL(TOKEN_POSITIONAL, token.type);
    TEST_ASSERT_EQUAL_STRING("input.txt", token.value);
}

void test_tokenize_positional_starts_with_dash(void) {
    token_t token = clap_tokenize_arg("-");
    /* Single dash is a short option */
    TEST_ASSERT_EQUAL(TOKEN_POSITIONAL, token.type);
    TEST_ASSERT_EQUAL_STRING("-", token.value);
}

void test_tokenize_stop(void) {
    token_t token = clap_tokenize_arg("--");
    TEST_ASSERT_EQUAL(TOKEN_STOP, token.type);
}

/* ============================================================================
 * Tokenize Tests - Long Options
 * ============================================================================ */

void test_tokenize_long_option(void) {
    token_t token = clap_tokenize_arg("--help");
    TEST_ASSERT_EQUAL(TOKEN_LONG_OPTION, token.type);
    TEST_ASSERT_EQUAL_STRING("help", token.option_name);
}

void test_tokenize_long_option_with_equals(void) {
    token_t token = clap_tokenize_arg("--output=file.txt");
    TEST_ASSERT_EQUAL(TOKEN_LONG_OPTION_EQ, token.type);
    TEST_ASSERT_EQUAL_STRING("output", token.option_name);
    TEST_ASSERT_EQUAL_STRING("file.txt", token.value);
    if (token.name_allocated) clap_free((void*)token.option_name);
}

void test_tokenize_long_option_with_multiple_equals(void) {
    token_t token = clap_tokenize_arg("--config=a=b=c");
    TEST_ASSERT_EQUAL(TOKEN_LONG_OPTION_EQ, token.type);
    /* option_name points to "config=a=b=c", caller handles '=' */
    TEST_ASSERT_EQUAL_STRING("a=b=c", token.value);
    if (token.name_allocated) clap_free((void*)token.option_name);
}

/* ============================================================================
 * Tokenize Tests - Short Options
 * ============================================================================ */

void test_tokenize_short_option(void) {
    token_t token = clap_tokenize_arg("-h");
    TEST_ASSERT_EQUAL(TOKEN_SHORT_OPTION, token.type);
    TEST_ASSERT_EQUAL_STRING("h", token.option_name);
}

void test_tokenize_short_option_bundle(void) {
    token_t token = clap_tokenize_arg("-abc");
    TEST_ASSERT_EQUAL(TOKEN_SHORT_OPTION_BUNDLE, token.type);
    TEST_ASSERT_EQUAL_STRING("abc", token.option_name);
}

void test_tokenize_short_option_bundle_two(void) {
    token_t token = clap_tokenize_arg("-xv");
    TEST_ASSERT_EQUAL(TOKEN_SHORT_OPTION_BUNDLE, token.type);
    TEST_ASSERT_EQUAL_STRING("xv", token.option_name);
}

/* ============================================================================
 * Expand Short Bundle Tests
 * ============================================================================ */

void test_expand_short_bundle_basic(void) {
    size_t count = 0;
    char **expanded = clap_expand_short_bundle("abc", &count);
    
    TEST_ASSERT_NOT_NULL(expanded);
    TEST_ASSERT_EQUAL(3, count);
    TEST_ASSERT_EQUAL_STRING("-a", expanded[0]);
    TEST_ASSERT_EQUAL_STRING("-b", expanded[1]);
    TEST_ASSERT_EQUAL_STRING("-c", expanded[2]);
    
    for (size_t i = 0; i < count; i++) {
        clap_free(expanded[i]);
    }
    clap_free(expanded);
}

void test_expand_short_bundle_single(void) {
    size_t count = 0;
    char **expanded = clap_expand_short_bundle("x", &count);
    
    TEST_ASSERT_NOT_NULL(expanded);
    TEST_ASSERT_EQUAL(1, count);
    TEST_ASSERT_EQUAL_STRING("-x", expanded[0]);
    
    clap_free(expanded[0]);
    clap_free(expanded);
}

void test_expand_short_bundle_empty(void) {
    size_t count = 0;
    char **expanded = clap_expand_short_bundle("", &count);
    
    TEST_ASSERT_NOT_NULL(expanded);
    TEST_ASSERT_EQUAL(0, count);
    
    clap_free(expanded);
}

/* ============================================================================
 * Check Required Option Tests
 * ============================================================================ */

void test_check_required_option_not_required(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--opt");
    clap_namespace_t *ns = clap_namespace_new();
    clap_error_t error = {0};

    bool result = check_required_option(arg, ns, &error);
    TEST_ASSERT_TRUE(result);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_check_required_option_missing_string(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--required");
    clap_argument_required(arg, true);
    clap_namespace_t *ns = clap_namespace_new();
    clap_error_t error = {0};

    bool result = check_required_option(arg, ns, &error);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_REQUIRED_MISSING, error.code);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_check_required_option_present_string(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--required");
    clap_argument_required(arg, true);
    clap_namespace_t *ns = clap_namespace_new();
    clap_namespace_set_string(ns, "required", "value");
    clap_error_t error = {0};

    bool result = check_required_option(arg, ns, &error);
    TEST_ASSERT_TRUE(result);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_check_required_option_missing_store_true(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--flag");
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    clap_argument_required(arg, true);
    clap_namespace_t *ns = clap_namespace_new();
    clap_error_t error = {0};

    bool result = check_required_option(arg, ns, &error);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_REQUIRED_MISSING, error.code);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_check_required_option_present_store_true(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--flag");
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    clap_argument_required(arg, true);
    clap_namespace_t *ns = clap_namespace_new();
    clap_namespace_set_bool(ns, "flag", true);
    clap_error_t error = {0};

    bool result = check_required_option(arg, ns, &error);
    TEST_ASSERT_TRUE(result);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

/* ============================================================================
 * Parse Args Integration Tests
 * ============================================================================ */

void test_parse_args_no_arguments(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    char *argv[] = {"prog"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    clap_parse_result_t result = clap_parse_args(parser, 1, argv, &ns, &error);
    
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);
    TEST_ASSERT_NOT_NULL(ns);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_parse_args_positional(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_add_argument(parser, "input");
    char *argv[] = {"prog", "file.txt"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    clap_parse_result_t result = clap_parse_args(parser, 2, argv, &ns, &error);
    
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);
    const char *value;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "input", &value));
    TEST_ASSERT_EQUAL_STRING("file.txt", value);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_parse_args_optional_store_true(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--verbose");
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    char *argv[] = {"prog", "--verbose"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    clap_parse_result_t result = clap_parse_args(parser, 2, argv, &ns, &error);
    
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);
    bool value;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "verbose", &value));
    TEST_ASSERT_TRUE(value);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_parse_args_optional_with_value(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--output");
    clap_argument_type(arg, "string");
    char *argv[] = {"prog", "--output", "out.txt"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    clap_parse_result_t result = clap_parse_args(parser, 3, argv, &ns, &error);
    
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);
    const char *value;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "output", &value));
    TEST_ASSERT_EQUAL_STRING("out.txt", value);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_parse_args_optional_with_equals(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--output");
    clap_argument_type(arg, "string");
    char *argv[] = {"prog", "--output=out.txt"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    clap_parse_result_t result = clap_parse_args(parser, 2, argv, &ns, &error);
    
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);
    const char *value;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "output", &value));
    TEST_ASSERT_EQUAL_STRING("out.txt", value);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_parse_args_short_option_bundle(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *a = clap_add_argument(parser, "-a");
    clap_argument_action(a, CLAP_ACTION_STORE_TRUE);
    clap_argument_t *b = clap_add_argument(parser, "-b");
    clap_argument_action(b, CLAP_ACTION_STORE_TRUE);
    clap_argument_t *c = clap_add_argument(parser, "-c");
    clap_argument_action(c, CLAP_ACTION_STORE_TRUE);
    
    char *argv[] = {"prog", "-abc"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    clap_parse_result_t result = clap_parse_args(parser, 2, argv, &ns, &error);
    
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);
    bool va, vb, vc;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "a", &va)); TEST_ASSERT_TRUE(va);
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "b", &vb)); TEST_ASSERT_TRUE(vb);
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "c", &vc)); TEST_ASSERT_TRUE(vc);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_parse_args_unrecognized_option(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    char *argv[] = {"prog", "--unknown"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    clap_parse_result_t result = clap_parse_args(parser, 2, argv, &ns, &error);
    
    TEST_ASSERT_EQUAL(CLAP_PARSE_ERROR, result);
    TEST_ASSERT_EQUAL(CLAP_ERR_UNRECOGNIZED, error.code);
    
    clap_parser_free(parser);
}

void test_parse_args_required_missing(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--required");
    clap_argument_required(arg, true);
    char *argv[] = {"prog"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    clap_parse_result_t result = clap_parse_args(parser, 1, argv, &ns, &error);
    
    TEST_ASSERT_EQUAL(CLAP_PARSE_ERROR, result);
    TEST_ASSERT_EQUAL(CLAP_ERR_REQUIRED_MISSING, error.code);
    
    clap_parser_free(parser);
}

void test_parse_args_stop_parsing(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--verbose");
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    clap_add_argument(parser, "input");
    
    char *argv[] = {"prog", "--", "--verbose"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    clap_parse_result_t result = clap_parse_args(parser, 3, argv, &ns, &error);
    
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);
    /* --verbose should NOT be parsed as option, but as positional */
    const char *value;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "input", &value));
    TEST_ASSERT_EQUAL_STRING("--verbose", value);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

void run_test_tokenizer(void) {
    /* Tokenize Tests */
    RUN_TEST(test_tokenize_null);
    RUN_TEST(test_tokenize_positional);
    RUN_TEST(test_tokenize_positional_starts_with_dash);
    RUN_TEST(test_tokenize_stop);
    RUN_TEST(test_tokenize_long_option);
    RUN_TEST(test_tokenize_long_option_with_equals);
    RUN_TEST(test_tokenize_long_option_with_multiple_equals);
    RUN_TEST(test_tokenize_short_option);
    RUN_TEST(test_tokenize_short_option_bundle);
    RUN_TEST(test_tokenize_short_option_bundle_two);
    
    /* Expand Short Bundle Tests */
    RUN_TEST(test_expand_short_bundle_basic);
    RUN_TEST(test_expand_short_bundle_single);
    RUN_TEST(test_expand_short_bundle_empty);
    
    /* Check Required Option Tests */
    RUN_TEST(test_check_required_option_not_required);
    RUN_TEST(test_check_required_option_missing_string);
    RUN_TEST(test_check_required_option_present_string);
    RUN_TEST(test_check_required_option_missing_store_true);
    RUN_TEST(test_check_required_option_present_store_true);
    
    /* Parse Args Integration Tests */
    RUN_TEST(test_parse_args_no_arguments);
    RUN_TEST(test_parse_args_positional);
    RUN_TEST(test_parse_args_optional_store_true);
    RUN_TEST(test_parse_args_optional_with_value);
    RUN_TEST(test_parse_args_optional_with_equals);
    RUN_TEST(test_parse_args_short_option_bundle);
    RUN_TEST(test_parse_args_unrecognized_option);
    RUN_TEST(test_parse_args_required_missing);
    RUN_TEST(test_parse_args_stop_parsing);
}

#ifdef STANDALONE_TEST
int main(void) {
    UNITY_BEGIN();
    run_test_tokenizer();
    return UNITY_END();
}
#endif
