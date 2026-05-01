/**
 * @file test_parse_flow.c
 * @brief End-to-end parsing flow tests
 */

#include "unity.h"
#include <clap/clap.h>
#include "../src/clap_parser_internal.h"
#include <string.h>

/* ============================================================================
 * Test Setup and Teardown
 * ============================================================================ */

void setUp(void) {}
void tearDown(void) {}

/* ============================================================================
 * Basic Parse Flow Tests
 * ============================================================================ */

void test_parse_flow_no_arguments(void) {
    clap_parser_t *parser = clap_parser_new("prog", "Test program", NULL);
    
    char *argv[] = {"prog"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 1, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_NOT_NULL(ns);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_parse_flow_help_option(void) {
    /* Help option exits, so we test by capturing or verifying it's present */
    clap_parser_t *parser = clap_parser_new("prog", "Test program", NULL);
    
    /* Verify help option exists */
    TEST_ASSERT_EQUAL(1, parser->arg_count);
    TEST_ASSERT_EQUAL(CLAP_ACTION_HELP, parser->arguments[0]->action);
    
    clap_parser_free(parser);
}

void test_parse_flow_positional_only(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_add_argument(parser, "input");
    clap_add_argument(parser, "output");
    
    char *argv[] = {"prog", "in.txt", "out.txt"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 3, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    
    const char *input, *output;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "input", &input));
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "output", &output));
    TEST_ASSERT_EQUAL_STRING("in.txt", input);
    TEST_ASSERT_EQUAL_STRING("out.txt", output);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_parse_flow_optional_only(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    clap_argument_t *verbose = clap_add_argument(parser, "--verbose");
    clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);
    
    clap_argument_t *output = clap_add_argument(parser, "--output");
    clap_argument_type(output, "string");
    
    char *argv[] = {"prog", "--verbose", "--output", "file.txt"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 4, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    
    bool verbose_val;
    const char *output_val;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "verbose", &verbose_val));
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "output", &output_val));
    TEST_ASSERT_TRUE(verbose_val);
    TEST_ASSERT_EQUAL_STRING("file.txt", output_val);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_parse_flow_mixed_arguments(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    clap_argument_t *verbose = clap_add_argument(parser, "--verbose");
    clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);
    
    clap_argument_t *output = clap_add_argument(parser, "--output");
    clap_argument_type(output, "string");
    
    clap_add_argument(parser, "input");
    
    char *argv[] = {"prog", "--output", "out.txt", "in.txt"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 4, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    
    const char *output_val, *input_val;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "output", &output_val));
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "input", &input_val));
    TEST_ASSERT_EQUAL_STRING("out.txt", output_val);
    TEST_ASSERT_EQUAL_STRING("in.txt", input_val);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

/* ============================================================================
 * Error Flow Tests
 * ============================================================================ */

void test_parse_flow_unrecognized_option(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    char *argv[] = {"prog", "--unknown"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 2, argv, &ns, &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_UNRECOGNIZED, error.code);
    TEST_ASSERT_NOT_NULL(strstr(error.message, "Unrecognized"));
    
    clap_parser_free(parser);
}

void test_parse_flow_missing_required(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    clap_argument_t *arg = clap_add_argument(parser, "--required");
    clap_argument_required(arg, true);
    
    char *argv[] = {"prog"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 1, argv, &ns, &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_REQUIRED_MISSING, error.code);
    
    clap_parser_free(parser);
}

void test_parse_flow_invalid_choice(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    clap_argument_t *color = clap_add_argument(parser, "--color");
    clap_argument_choices(color, (const char*[]){"red", "green", "blue"}, 3);
    
    char *argv[] = {"prog", "--color", "yellow"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 3, argv, &ns, &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_INVALID_CHOICE, error.code);
    TEST_ASSERT_NOT_NULL(strstr(error.message, "yellow"));
    
    clap_parser_free(parser);
}

void test_parse_flow_type_conversion_error(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    clap_argument_t *age = clap_add_argument(parser, "--age");
    clap_argument_type(age, "int");
    
    char *argv[] = {"prog", "--age", "abc"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 3, argv, &ns, &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_TYPE_CONVERSION, error.code);
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Mutex Group Flow Tests
 * ============================================================================ */

void test_parse_flow_mutex_conflict(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group = clap_add_mutually_exclusive_group(parser, false);
    
    clap_argument_t *verbose = clap_add_argument(parser, "--verbose");
    clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);
    clap_argument_mutex_group(verbose, group);
    clap_mutex_group_add_argument(parser, group, verbose);
    
    clap_argument_t *quiet = clap_add_argument(parser, "--quiet");
    clap_argument_action(quiet, CLAP_ACTION_STORE_TRUE);
    clap_argument_mutex_group(quiet, group);
    clap_mutex_group_add_argument(parser, group, quiet);
    
    char *argv[] = {"prog", "--verbose", "--quiet"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 3, argv, &ns, &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_MUTUALLY_EXCLUSIVE, error.code);
    
    clap_parser_free(parser);
}

void test_parse_flow_mutex_required_missing(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group = clap_add_mutually_exclusive_group(parser, true);
    
    clap_argument_t *start = clap_add_argument(parser, "--start");
    clap_argument_mutex_group(start, group);
    clap_mutex_group_add_argument(parser, group, start);
    
    clap_argument_t *stop = clap_add_argument(parser, "--stop");
    clap_argument_mutex_group(stop, group);
    clap_mutex_group_add_argument(parser, group, stop);
    
    char *argv[] = {"prog"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 1, argv, &ns, &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_REQUIRED_MISSING, error.code);
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Subcommand Flow Tests
 * ============================================================================ */

void test_parse_flow_subcommand_basic(void) {
    clap_parser_t *parser = clap_parser_new("git", NULL, NULL);
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    
    clap_parser_t *commit = clap_subparser_add(subparsers, "commit", NULL);
    clap_argument_t *msg = clap_add_argument(commit, "-m");
    clap_argument_dest(msg,"message");
    clap_argument_type(msg, "string");
    
    char *argv[] = {"git", "commit", "-m", "hello"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 4, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    
    const char *cmd;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "cmd", &cmd));
    TEST_ASSERT_EQUAL_STRING("commit", cmd);
    
    const char *message;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "message", &message));
    TEST_ASSERT_EQUAL_STRING("hello", message);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_parse_flow_subcommand_with_global(void) {
    clap_parser_t *parser = clap_parser_new("git", NULL, NULL);
    
    clap_argument_t *verbose = clap_add_argument(parser, "--verbose");
    clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);
    
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    clap_parser_t *commit = clap_subparser_add(subparsers, "commit", NULL);
    clap_argument_t *msg = clap_add_argument(commit, "-m");
    clap_argument_dest(msg,"message");
    clap_argument_type(msg, "string");
    
    char *argv[] = {"git", "--verbose", "commit", "-m", "hello"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 5, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    
    bool verbose_val;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "verbose", &verbose_val));
    TEST_ASSERT_TRUE(verbose_val);
    
    const char *cmd;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "cmd", &cmd));
    TEST_ASSERT_EQUAL_STRING("commit", cmd);
    
    const char *message;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "message", &message));
    TEST_ASSERT_EQUAL_STRING("hello", message);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_parse_flow_subcommand_error(void) {
    clap_parser_t *parser = clap_parser_new("git", NULL, NULL);
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    
    clap_parser_t *commit = clap_subparser_add(subparsers, "commit", NULL);
    clap_argument_t *msg = clap_add_argument(commit, "-m");
    clap_argument_required(msg, true);
    
    char *argv[] = {"git", "commit"};  /* Missing -m */
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 2, argv, &ns, &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_REQUIRED_MISSING, error.code);
    TEST_ASSERT_EQUAL_STRING("commit", error.subcommand_name);
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Nargs Flow Tests
 * ============================================================================ */

void test_parse_flow_nargs_star(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *files = clap_add_argument(parser, "files");
    clap_argument_nargs(files, '*');
    
    char *argv[] = {"prog", "a.txt", "b.txt", "c.txt"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 4, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    
    const char **file_list;
    size_t count;
    TEST_ASSERT_TRUE(clap_namespace_get_string_array(ns, "files", &file_list, &count));
    TEST_ASSERT_EQUAL(3, count);
    TEST_ASSERT_EQUAL_STRING("a.txt", file_list[0]);
    TEST_ASSERT_EQUAL_STRING("b.txt", file_list[1]);
    TEST_ASSERT_EQUAL_STRING("c.txt", file_list[2]);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_parse_flow_nargs_plus(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *files = clap_add_argument(parser, "files");
    clap_argument_nargs(files, '+');
    
    char *argv[] = {"prog", "a.txt", "b.txt"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};

    bool result = clap_parse_args(parser, 3, argv, &ns, &error);
    TEST_ASSERT_TRUE(result);
    
    clap_namespace_free(ns);
    
    /* Test failure with 0 arguments */
    char *argv2[] = {"prog"};
    result = clap_parse_args(parser, 1, argv2, &ns, &error);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_REQUIRED_MISSING, error.code);
    
    clap_parser_free(parser);
}

void test_parse_flow_stop_parsing(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *verbose = clap_add_argument(parser, "--verbose");
    clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);
    clap_add_argument(parser, "input");
    
    char *argv[] = {"prog", "--", "--verbose"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 3, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    
    const char *input;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "input", &input));
    TEST_ASSERT_EQUAL_STRING("--verbose", input);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

/* ============================================================================
 * nargs=N Tests - Positional Arguments
 * ============================================================================ */

/**
 * @brief Test: Positional nargs=3 with exactly 3 arguments - should succeed
 */
void test_parse_flow_positional_nargs_exact_success(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *nums = clap_add_argument(parser, "nums");
    clap_argument_nargs(nums, 3);

    char *argv[] = {"prog", "1", "2", "3"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};

    bool result = clap_parse_args(parser, 4, argv, &ns, &error);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_NONE, error.code);

    const char **values;
    size_t count;
    TEST_ASSERT_TRUE(clap_namespace_get_string_array(ns, "nums", &values, &count));
    TEST_ASSERT_EQUAL(3, count);
    TEST_ASSERT_EQUAL_STRING("1", values[0]);
    TEST_ASSERT_EQUAL_STRING("2", values[1]);
    TEST_ASSERT_EQUAL_STRING("3", values[2]);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

/**
 * @brief Test: Positional nargs=3 with 2 arguments - should fail with TOO_FEW_ARGS
 */
void test_parse_flow_positional_nargs_too_few(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *nums = clap_add_argument(parser, "nums");
    clap_argument_nargs(nums, 3);

    char *argv[] = {"prog", "1", "2"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};

    bool result = clap_parse_args(parser, 3, argv, &ns, &error);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_TOO_FEW_ARGS, error.code);
    TEST_ASSERT_NOT_NULL(strstr(error.message, "nums"));
    TEST_ASSERT_NULL(ns);

    clap_parser_free(parser);
}

/**
 * @brief Test: Positional nargs=3 with 0 arguments - should fail with REQUIRED_MISSING
 */
void test_parse_flow_positional_nargs_zero_provided(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *nums = clap_add_argument(parser, "nums");
    clap_argument_nargs(nums, 3);

    char *argv[] = {"prog"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};

    bool result = clap_parse_args(parser, 1, argv, &ns, &error);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_REQUIRED_MISSING, error.code);
    TEST_ASSERT_NOT_NULL(strstr(error.message, "nums"));
    TEST_ASSERT_NULL(ns);

    clap_parser_free(parser);
}

/**
 * @brief Test: Positional nargs=3 with 4 arguments - should fail with TOO_MANY_ARGS
 */
void test_parse_flow_positional_nargs_too_many(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *nums = clap_add_argument(parser, "nums");
    clap_argument_nargs(nums, 3);

    char *argv[] = {"prog", "1", "2", "3", "4"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};

    bool result = clap_parse_args(parser, 5, argv, &ns, &error);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_TOO_MANY_ARGS, error.code);
    TEST_ASSERT_NULL(ns);

    clap_parser_free(parser);
}

/**
 * @brief Test: Positional nargs=3 with optional argument before it
 */
void test_parse_flow_positional_nargs_with_optional_before(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);

    clap_argument_t *verbose = clap_add_argument(parser, "--verbose");
    clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);

    clap_argument_t *nums = clap_add_argument(parser, "nums");
    clap_argument_nargs(nums, 3);

    char *argv[] = {"prog", "--verbose", "1", "2", "3"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};

    bool result = clap_parse_args(parser, 5, argv, &ns, &error);
    TEST_ASSERT_TRUE(result);

    bool verbose_val;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "verbose", &verbose_val));
    TEST_ASSERT_TRUE(verbose_val);

    const char **values;
    size_t count;
    TEST_ASSERT_TRUE(clap_namespace_get_string_array(ns, "nums", &values, &count));
    TEST_ASSERT_EQUAL(3, count);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

/* ============================================================================
 * nargs=N Tests - Optional Arguments
 * ============================================================================ */

/**
 * @brief Test: Optional with nargs=3, exactly 3 provided - success
 */
void test_parse_flow_optional_nargs_exact_success(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *nums = clap_add_argument(parser, "--nums");
    clap_argument_nargs(nums, 3);

    char *argv[] = {"prog", "--nums", "1", "2", "3"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};

    bool result = clap_parse_args(parser, 5, argv, &ns, &error);
    TEST_ASSERT_TRUE(result);

    const char **values;
    size_t count;
    TEST_ASSERT_TRUE(clap_namespace_get_string_array(ns, "nums", &values, &count));
    TEST_ASSERT_EQUAL(3, count);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

/**
 * @brief Test: Optional with nargs=3, 2 provided - TOO_FEW_ARGS
 */
void test_parse_flow_optional_nargs_too_few(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *nums = clap_add_argument(parser, "--nums");
    clap_argument_nargs(nums, 3);

    char *argv[] = {"prog", "--nums", "1", "2"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};

    bool result = clap_parse_args(parser, 4, argv, &ns, &error);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_TOO_FEW_ARGS, error.code);

    clap_parser_free(parser);
}

/**
 * @brief Test: Optional with nargs=3, 0 provided - SUCCESS (optional)
 */
void test_parse_flow_optional_nargs_zero_provided(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *nums = clap_add_argument(parser, "--nums");
    clap_argument_nargs(nums, 3);

    char *argv[] = {"prog"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};

    bool result = clap_parse_args(parser, 1, argv, &ns, &error);
    TEST_ASSERT_TRUE(result);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

/**
 * @brief Test: Optional with nargs='+', 0 provided - SUCCESS (optional)
 */
void test_parse_flow_optional_nargs_plus_zero_provided(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *files = clap_add_argument(parser, "--files");
    clap_argument_nargs(files, '+');

    char *argv[] = {"prog"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};

    bool result = clap_parse_args(parser, 1, argv, &ns, &error);
    TEST_ASSERT_TRUE(result);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

/**
 * @brief Test: Required optional with nargs=3, 0 provided - REQUIRED_MISSING
 */
void test_parse_flow_required_optional_nargs_zero(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *nums = clap_add_argument(parser, "--nums");
    clap_argument_nargs(nums, 3);
    clap_argument_required(nums, true);

    char *argv[] = {"prog"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};

    bool result = clap_parse_args(parser, 1, argv, &ns, &error);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_REQUIRED_MISSING, error.code);

    clap_parser_free(parser);
}

/**
 * @brief Test: Required optional with nargs='+', 0 provided - REQUIRED_MISSING
 */
void test_parse_flow_required_optional_nargs_plus_zero(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *files = clap_add_argument(parser, "--files");
    clap_argument_nargs(files, '+');
    clap_argument_required(files, true);

    char *argv[] = {"prog"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};

    bool result = clap_parse_args(parser, 1, argv, &ns, &error);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_REQUIRED_MISSING, error.code);

    clap_parser_free(parser);
}

/* ============================================================================
 * Short Option Bundle Flow Tests
 * ============================================================================ */

void test_parse_flow_short_option_bundle(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    clap_argument_t *a_opt = clap_add_argument(parser, "-a");
    clap_argument_action(a_opt, CLAP_ACTION_STORE_TRUE);
    
    clap_argument_t *b_opt = clap_add_argument(parser, "-b");
    clap_argument_action(b_opt, CLAP_ACTION_STORE_TRUE);
    
    clap_argument_t *c_opt = clap_add_argument(parser, "-c");
    clap_argument_action(c_opt, CLAP_ACTION_STORE_TRUE);
    
    char *argv[] = {"prog", "-abc"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 2, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_NONE, error.code);
    
    bool a_val, b_val, c_val;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "a", &a_val));
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "b", &b_val));
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "c", &c_val));
    TEST_ASSERT_TRUE(a_val);
    TEST_ASSERT_TRUE(b_val);
    TEST_ASSERT_TRUE(c_val);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_parse_flow_short_option_bundle_mixed_with_positional(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    clap_argument_t *a_opt = clap_add_argument(parser, "-a");
    clap_argument_action(a_opt, CLAP_ACTION_STORE_TRUE);
    
    clap_argument_t *b_opt = clap_add_argument(parser, "-b");
    clap_argument_action(b_opt, CLAP_ACTION_STORE_TRUE);
    
    clap_add_argument(parser, "input");
    clap_add_argument(parser, "output");
    
    char *argv[] = {"prog", "-ab", "in.txt", "out.txt"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 4, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_NONE, error.code);
    
    bool a_val, b_val;
    const char *input, *output;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "a", &a_val));
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "b", &b_val));
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "input", &input));
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "output", &output));
    TEST_ASSERT_TRUE(a_val);
    TEST_ASSERT_TRUE(b_val);
    TEST_ASSERT_EQUAL_STRING("in.txt", input);
    TEST_ASSERT_EQUAL_STRING("out.txt", output);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_parse_flow_short_option_bundle_unrecognized(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    clap_argument_t *a_opt = clap_add_argument(parser, "-a");
    clap_argument_action(a_opt, CLAP_ACTION_STORE_TRUE);
    
    char *argv[] = {"prog", "-ab"};  // -b not defined
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 2, argv, &ns, &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_UNRECOGNIZED, error.code);
    TEST_ASSERT_NULL(ns);
    
    clap_parser_free(parser);
}

void test_parse_flow_short_option_bundle_multiple(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    clap_argument_t *a_opt = clap_add_argument(parser, "-a");
    clap_argument_action(a_opt, CLAP_ACTION_STORE_TRUE);
    
    clap_argument_t *b_opt = clap_add_argument(parser, "-b");
    clap_argument_action(b_opt, CLAP_ACTION_STORE_TRUE);
    
    clap_argument_t *c_opt = clap_add_argument(parser, "-c");
    clap_argument_action(c_opt, CLAP_ACTION_STORE_TRUE);
    
    clap_argument_t *d_opt = clap_add_argument(parser, "-d");
    clap_argument_action(d_opt, CLAP_ACTION_STORE_TRUE);
    
    char *argv[] = {"prog", "-ab", "-cd"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 3, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_NONE, error.code);
    
    bool a_val, b_val, c_val, d_val;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "a", &a_val));
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "b", &b_val));
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "c", &c_val));
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "d", &d_val));
    TEST_ASSERT_TRUE(a_val);
    TEST_ASSERT_TRUE(b_val);
    TEST_ASSERT_TRUE(c_val);
    TEST_ASSERT_TRUE(d_val);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_parse_flow_short_option_bundle_with_long_option(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    clap_argument_t *a_opt = clap_add_argument(parser, "-a");
    clap_argument_action(a_opt, CLAP_ACTION_STORE_TRUE);
    
    clap_argument_t *b_opt = clap_add_argument(parser, "-b");
    clap_argument_action(b_opt, CLAP_ACTION_STORE_TRUE);
    
    clap_argument_t *verbose = clap_add_argument(parser, "--verbose");
    clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);
    
    char *argv[] = {"prog", "-ab", "--verbose"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 3, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_NONE, error.code);
    
    bool a_val, b_val, verbose_val;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "a", &a_val));
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "b", &b_val));
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "verbose", &verbose_val));
    TEST_ASSERT_TRUE(a_val);
    TEST_ASSERT_TRUE(b_val);
    TEST_ASSERT_TRUE(verbose_val);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_parse_flow_short_option_with_value(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    clap_argument_t *file_opt = clap_add_argument(parser, "-f");
    clap_argument_dest(file_opt, "file");
    clap_argument_type(file_opt, "string");
    
    char *argv[] = {"prog", "-f", "input.txt"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 3, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_NONE, error.code);
    
    const char *file_val;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "file", &file_val));
    TEST_ASSERT_EQUAL_STRING("input.txt", file_val);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_parse_flow_long_option_with_value(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    clap_argument_t *file_opt = clap_add_argument(parser, "--file");
    clap_argument_type(file_opt, "string");
    
    char *argv[] = {"prog", "--file", "output.txt"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 3, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_NONE, error.code);
    
    const char *file_val;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "file", &file_val));
    TEST_ASSERT_EQUAL_STRING("output.txt", file_val);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_parse_flow_option_append(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    clap_argument_t *include_opt = clap_add_argument(parser, "--include");
    clap_argument_action(include_opt, CLAP_ACTION_APPEND);
    clap_argument_type(include_opt, "string");
    
    char *argv[] = {"prog", "--include", "file1.h", "--include", "file2.h"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 5, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_NONE, error.code);
    
    const char **includes;
    size_t count;
    TEST_ASSERT_TRUE(clap_namespace_get_string_array(ns, "include", &includes, &count));
    TEST_ASSERT_EQUAL(2, count);
    TEST_ASSERT_EQUAL_STRING("file1.h", includes[0]);
    TEST_ASSERT_EQUAL_STRING("file2.h", includes[1]);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_parse_flow_no_arguments);
    RUN_TEST(test_parse_flow_help_option);
    RUN_TEST(test_parse_flow_positional_only);
    RUN_TEST(test_parse_flow_optional_only);
    RUN_TEST(test_parse_flow_mixed_arguments);

    RUN_TEST(test_parse_flow_unrecognized_option);
    RUN_TEST(test_parse_flow_missing_required);
    RUN_TEST(test_parse_flow_invalid_choice);
    RUN_TEST(test_parse_flow_type_conversion_error);

    RUN_TEST(test_parse_flow_mutex_conflict);
    RUN_TEST(test_parse_flow_mutex_required_missing);

    RUN_TEST(test_parse_flow_subcommand_basic);
    RUN_TEST(test_parse_flow_subcommand_with_global);
    RUN_TEST(test_parse_flow_subcommand_error);

    RUN_TEST(test_parse_flow_nargs_star);
    RUN_TEST(test_parse_flow_nargs_plus);
    RUN_TEST(test_parse_flow_stop_parsing);

    // /* Positional nargs=N tests */
    RUN_TEST(test_parse_flow_positional_nargs_exact_success);
    RUN_TEST(test_parse_flow_positional_nargs_too_few);
    RUN_TEST(test_parse_flow_positional_nargs_zero_provided);
    RUN_TEST(test_parse_flow_positional_nargs_too_many);
    RUN_TEST(test_parse_flow_positional_nargs_with_optional_before);

    // /* Optional nargs=N tests */
    RUN_TEST(test_parse_flow_optional_nargs_exact_success);
    RUN_TEST(test_parse_flow_optional_nargs_too_few);
    RUN_TEST(test_parse_flow_optional_nargs_zero_provided);
    RUN_TEST(test_parse_flow_optional_nargs_plus_zero_provided);
    RUN_TEST(test_parse_flow_required_optional_nargs_zero);
    RUN_TEST(test_parse_flow_required_optional_nargs_plus_zero);

    RUN_TEST(test_parse_flow_short_option_bundle);
    RUN_TEST(test_parse_flow_short_option_bundle_mixed_with_positional);
    RUN_TEST(test_parse_flow_short_option_bundle_unrecognized);
    RUN_TEST(test_parse_flow_short_option_bundle_multiple);
    RUN_TEST(test_parse_flow_short_option_bundle_with_long_option);
    RUN_TEST(test_parse_flow_short_option_with_value);
    RUN_TEST(test_parse_flow_long_option_with_value);
    RUN_TEST(test_parse_flow_option_append);

    return UNITY_END();
}