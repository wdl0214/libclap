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
 * clap_tokenize Tests (multi-arg tokenizer)
 * ============================================================================ */

void test_tokenize_no_args(void) {
    char *argv[] = {"prog"};
    size_t token_count = 0;
    clap_error_t error = {0};

    clap_token_t *tokens = clap_tokenize(1, argv, &token_count, &error);
    TEST_ASSERT_NULL(tokens);
    TEST_ASSERT_EQUAL(0, token_count);
}

void test_tokenize_single_token(void) {
    char *argv[] = {"prog", "--verbose"};
    size_t token_count = 0;
    clap_error_t error = {0};

    clap_token_t *tokens = clap_tokenize(2, argv, &token_count, &error);
    TEST_ASSERT_NOT_NULL(tokens);
    TEST_ASSERT_EQUAL(1, token_count);
    TEST_ASSERT_EQUAL(TOKEN_LONG_OPTION, tokens[0].type);
    TEST_ASSERT_EQUAL_STRING("verbose", tokens[0].option_name);

    clap_tokenize_free(tokens, token_count);
}

void test_tokenize_multiple_token_types(void) {
    char *argv[] = {"prog", "-v", "--output", "result.txt", "--", "extra.txt"};
    size_t token_count = 0;
    clap_error_t error = {0};

    clap_token_t *tokens = clap_tokenize(6, argv, &token_count, &error);
    TEST_ASSERT_NOT_NULL(tokens);
    TEST_ASSERT_EQUAL(5, token_count);
    TEST_ASSERT_EQUAL(TOKEN_SHORT_OPTION, tokens[0].type);
    TEST_ASSERT_EQUAL_STRING("v", tokens[0].option_name);
    TEST_ASSERT_EQUAL(TOKEN_LONG_OPTION, tokens[1].type);
    TEST_ASSERT_EQUAL_STRING("output", tokens[1].option_name);
    TEST_ASSERT_EQUAL(TOKEN_POSITIONAL, tokens[2].type);
    TEST_ASSERT_EQUAL_STRING("result.txt", tokens[2].value);
    TEST_ASSERT_EQUAL(TOKEN_STOP, tokens[3].type);
    TEST_ASSERT_EQUAL(TOKEN_POSITIONAL, tokens[4].type);
    TEST_ASSERT_EQUAL_STRING("extra.txt", tokens[4].value);

    clap_tokenize_free(tokens, token_count);
}

void test_tokenize_long_option_with_equals_in_batch(void) {
    char *argv[] = {"prog", "--config=debug.ini", "--output=result.txt"};
    size_t token_count = 0;
    clap_error_t error = {0};

    clap_token_t *tokens = clap_tokenize(3, argv, &token_count, &error);
    TEST_ASSERT_NOT_NULL(tokens);
    TEST_ASSERT_EQUAL(2, token_count);
    TEST_ASSERT_EQUAL(TOKEN_LONG_OPTION_EQ, tokens[0].type);
    TEST_ASSERT_EQUAL_STRING("config", tokens[0].option_name);
    TEST_ASSERT_EQUAL_STRING("debug.ini", tokens[0].value);
    TEST_ASSERT_EQUAL(TOKEN_LONG_OPTION_EQ, tokens[1].type);
    TEST_ASSERT_EQUAL_STRING("output", tokens[1].option_name);
    TEST_ASSERT_EQUAL_STRING("result.txt", tokens[1].value);

    clap_tokenize_free(tokens, token_count);
}

void test_tokenize_null_argv(void) {
    size_t token_count = 0;
    clap_error_t error = {0};
    clap_token_t *tokens = clap_tokenize(2, NULL, &token_count, &error);
    TEST_ASSERT_NULL(tokens);
    TEST_ASSERT_EQUAL(CLAP_ERR_INVALID_ARGUMENT, error.code);
}

void test_tokenize_null_count(void) {
    char *argv[] = {"prog", "arg"};
    clap_error_t error = {0};
    clap_token_t *tokens = clap_tokenize(2, argv, NULL, &error);
    TEST_ASSERT_NULL(tokens);
    TEST_ASSERT_EQUAL(CLAP_ERR_INVALID_ARGUMENT, error.code);
}

/* ============================================================================
 * clap_tokenize_free Tests
 * ============================================================================ */

void test_tokenize_free_basic(void) {
    char *argv[] = {"prog", "--config=value", "-v", "file.txt"};
    size_t token_count = 0;
    clap_error_t error = {0};
    clap_token_t *tokens = clap_tokenize(4, argv, &token_count, &error);
    TEST_ASSERT_NOT_NULL(tokens);

    /* Should not crash */
    clap_tokenize_free(tokens, token_count);
}

void test_tokenize_free_null(void) {
    /* Should not crash */
    clap_tokenize_free(NULL, 0);
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
 * Check Required Positional Tests
 * ============================================================================ */

void test_check_required_positional_nargs_one_present(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "input");
    clap_namespace_t *ns = clap_namespace_new();
    clap_namespace_set_string(ns, "input", "value");
    clap_error_t error = {0};

    bool result = check_required_positional(arg, ns, &error);
    TEST_ASSERT_TRUE(result);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_check_required_positional_nargs_one_missing(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "input");
    clap_namespace_t *ns = clap_namespace_new();
    clap_error_t error = {0};

    bool result = check_required_positional(arg, ns, &error);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_REQUIRED_MISSING, error.code);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_check_required_positional_nargs_question_not_required(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "input");
    arg->nargs = CLAP_NARGS_ZERO_OR_ONE;
    clap_namespace_t *ns = clap_namespace_new();
    clap_error_t error = {0};

    /* nargs='?' is never required */
    bool result = check_required_positional(arg, ns, &error);
    TEST_ASSERT_TRUE(result);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_check_required_positional_nargs_star_not_required(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "files");
    arg->nargs = CLAP_NARGS_ZERO_OR_MORE;
    clap_namespace_t *ns = clap_namespace_new();
    clap_error_t error = {0};

    /* nargs='*' is never required */
    bool result = check_required_positional(arg, ns, &error);
    TEST_ASSERT_TRUE(result);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_check_required_positional_nargs_remainder_not_required(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "rest");
    arg->nargs = CLAP_NARGS_REMAINDER;
    clap_namespace_t *ns = clap_namespace_new();
    clap_error_t error = {0};

    bool result = check_required_positional(arg, ns, &error);
    TEST_ASSERT_TRUE(result);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_check_required_positional_nargs_fixed_present(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "nums");
    arg->nargs = 3;
    clap_namespace_t *ns = clap_namespace_new();
    clap_namespace_append_string(ns, "nums", "1");
    clap_namespace_append_string(ns, "nums", "2");
    clap_namespace_append_string(ns, "nums", "3");
    clap_error_t error = {0};

    bool result = check_required_positional(arg, ns, &error);
    TEST_ASSERT_TRUE(result);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_check_required_positional_nargs_fixed_wrong_count(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "nums");
    arg->nargs = 3;
    clap_namespace_t *ns = clap_namespace_new();
    clap_namespace_append_string(ns, "nums", "1");
    clap_error_t error = {0};

    bool result = check_required_positional(arg, ns, &error);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_REQUIRED_MISSING, error.code);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

/* ============================================================================
 * Check Positional nargs Constraint Tests
 * ============================================================================ */

void test_check_positional_nargs_constraint_plus_satisfied(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "files");
    arg->nargs = CLAP_NARGS_ONE_OR_MORE;
    clap_namespace_t *ns = clap_namespace_new();
    clap_namespace_append_string(ns, "files", "a.txt");
    clap_namespace_append_string(ns, "files", "b.txt");
    clap_error_t error = {0};

    bool result = check_positional_nargs_constraint(arg, ns, &error);
    TEST_ASSERT_TRUE(result);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_check_positional_nargs_constraint_plus_unsatisfied(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "files");
    arg->nargs = CLAP_NARGS_ONE_OR_MORE;
    clap_namespace_t *ns = clap_namespace_new();
    clap_error_t error = {0};

    bool result = check_positional_nargs_constraint(arg, ns, &error);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_REQUIRED_MISSING, error.code);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_check_positional_nargs_constraint_fixed_n_exact(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "coords");
    arg->nargs = 3;
    clap_namespace_t *ns = clap_namespace_new();
    clap_namespace_append_string(ns, "coords", "x");
    clap_namespace_append_string(ns, "coords", "y");
    clap_namespace_append_string(ns, "coords", "z");
    clap_error_t error = {0};

    bool result = check_positional_nargs_constraint(arg, ns, &error);
    TEST_ASSERT_TRUE(result);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_check_positional_nargs_constraint_fixed_n_too_few(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "coords");
    arg->nargs = 3;
    clap_namespace_t *ns = clap_namespace_new();
    clap_namespace_append_string(ns, "coords", "x");
    clap_error_t error = {0};

    bool result = check_positional_nargs_constraint(arg, ns, &error);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_TOO_FEW_ARGS, error.code);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_check_positional_nargs_constraint_fixed_n_too_many(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "pair");
    arg->nargs = 2;
    clap_namespace_t *ns = clap_namespace_new();
    clap_namespace_append_string(ns, "pair", "a");
    clap_namespace_append_string(ns, "pair", "b");
    clap_namespace_append_string(ns, "pair", "c");
    clap_error_t error = {0};

    bool result = check_positional_nargs_constraint(arg, ns, &error);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_TOO_MANY_ARGS, error.code);

    clap_namespace_free(ns);
    clap_parser_free(parser);
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

    /* clap_tokenize Tests (multi-arg) */
    RUN_TEST(test_tokenize_no_args);
    RUN_TEST(test_tokenize_single_token);
    RUN_TEST(test_tokenize_multiple_token_types);
    RUN_TEST(test_tokenize_long_option_with_equals_in_batch);
    RUN_TEST(test_tokenize_null_argv);
    RUN_TEST(test_tokenize_null_count);

    /* clap_tokenize_free Tests */
    RUN_TEST(test_tokenize_free_basic);
    RUN_TEST(test_tokenize_free_null);

    /* Expand Short Bundle Tests */
    RUN_TEST(test_expand_short_bundle_basic);
    RUN_TEST(test_expand_short_bundle_single);
    RUN_TEST(test_expand_short_bundle_empty);

    /* Check Required Positional Tests */
    RUN_TEST(test_check_required_positional_nargs_one_present);
    RUN_TEST(test_check_required_positional_nargs_one_missing);
    RUN_TEST(test_check_required_positional_nargs_question_not_required);
    RUN_TEST(test_check_required_positional_nargs_star_not_required);
    RUN_TEST(test_check_required_positional_nargs_remainder_not_required);
    RUN_TEST(test_check_required_positional_nargs_fixed_present);
    RUN_TEST(test_check_required_positional_nargs_fixed_wrong_count);

    /* Check Positional nargs Constraint Tests */
    RUN_TEST(test_check_positional_nargs_constraint_plus_satisfied);
    RUN_TEST(test_check_positional_nargs_constraint_plus_unsatisfied);
    RUN_TEST(test_check_positional_nargs_constraint_fixed_n_exact);
    RUN_TEST(test_check_positional_nargs_constraint_fixed_n_too_few);
    RUN_TEST(test_check_positional_nargs_constraint_fixed_n_too_many);

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
