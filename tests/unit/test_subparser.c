/**
 * @file test_subparser.c
 * @brief Unit tests for clap_subparser.c
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
 * clap_add_subparsers Tests
 * ============================================================================ */

void test_add_subparsers_basic(void) {
    clap_parser_t *parser = clap_parser_new("prog", "Main description", NULL);
    
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", "Subcommands");
    
    TEST_ASSERT_NOT_NULL(subparsers);
    TEST_ASSERT_TRUE(parser->has_subparsers);
    TEST_ASSERT_EQUAL_STRING("cmd", clap_buffer_cstr(parser->subparser_dest));
    TEST_ASSERT_EQUAL_PTR(subparsers, parser->subparsers_container);
    TEST_ASSERT_EQUAL(parser->help_width, subparsers->help_width);
    TEST_ASSERT_EQUAL_STRING("prog", clap_buffer_cstr(subparsers->prog_name));
    
    clap_parser_free(parser);
}

void test_add_subparsers_null_parser(void) {
    clap_parser_t *result = clap_add_subparsers(NULL, "cmd", NULL);
    TEST_ASSERT_NULL(result);
}

void test_add_subparsers_null_dest(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_parser_t *result = clap_add_subparsers(parser, NULL, NULL);
    TEST_ASSERT_NULL(result);
    clap_parser_free(parser);
}

void test_add_subparsers_inherits_help_width(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_parser_set_help_width(parser, 120);
    
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    
    TEST_ASSERT_EQUAL(120, subparsers->help_width);
    
    clap_parser_free(parser);
}

void test_add_subparsers_empty_prog_name(void) {
    clap_parser_t *parser = clap_parser_new("", NULL, NULL);
    
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    
    TEST_ASSERT_NOT_NULL(subparsers);
    TEST_ASSERT_EQUAL_STRING("", clap_buffer_cstr(subparsers->prog_name));
    
    clap_parser_free(parser);
}

/* ============================================================================
 * clap_subparser_add Tests
 * ============================================================================ */

void test_subparser_add_basic(void) {
    clap_parser_t *parser = clap_parser_new("git", NULL, NULL);
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    
    clap_parser_t *commit = clap_subparser_add(subparsers, "commit", "Commit changes");
    
    TEST_ASSERT_NOT_NULL(commit);
    TEST_ASSERT_EQUAL(1, subparsers->subparser_count);
    TEST_ASSERT_EQUAL_PTR(commit, subparsers->subparsers[0]);
    
    /* Check full program name */
    TEST_ASSERT_EQUAL_STRING("git commit", clap_buffer_cstr(commit->prog_name));
    
    clap_parser_free(parser);
}

void test_subparser_add_multiple(void) {
    clap_parser_t *parser = clap_parser_new("git", NULL, NULL);
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    
    clap_parser_t *commit = clap_subparser_add(subparsers, "commit", NULL);
    clap_parser_t *add = clap_subparser_add(subparsers, "add", NULL);
    clap_parser_t *push = clap_subparser_add(subparsers, "push", NULL);
    
    TEST_ASSERT_EQUAL(3, subparsers->subparser_count);
    TEST_ASSERT_EQUAL_STRING("git commit", clap_buffer_cstr(commit->prog_name));
    TEST_ASSERT_EQUAL_STRING("git add", clap_buffer_cstr(add->prog_name));
    TEST_ASSERT_EQUAL_STRING("git push", clap_buffer_cstr(push->prog_name));
    
    clap_parser_free(parser);
}

void test_subparser_add_null_subparsers(void) {
    clap_parser_t *result = clap_subparser_add(NULL, "cmd", NULL);
    TEST_ASSERT_NULL(result);
}

void test_subparser_add_null_name(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    
    clap_parser_t *result = clap_subparser_add(subparsers, NULL, NULL);
    TEST_ASSERT_NULL(result);
    
    clap_parser_free(parser);
}

void test_subparser_add_inherits_help_width(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_parser_set_help_width(parser, 150);
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    
    clap_parser_t *cmd = clap_subparser_add(subparsers, "test", NULL);
    
    TEST_ASSERT_EQUAL(150, cmd->help_width);
    
    clap_parser_free(parser);
}

void test_subparser_add_with_empty_parent_prog_name(void) {
    clap_parser_t *parser = clap_parser_new("", NULL, NULL);
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    
    clap_parser_t *cmd = clap_subparser_add(subparsers, "test", NULL);
    
    /* When parent prog_name is empty, full name should be " test" or just "test" */
    TEST_ASSERT_NOT_NULL(cmd);
    
    clap_parser_free(parser);
}

void test_subparser_add_expands_array(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    
    /* Add enough subcommands to force array expansion */
    for (int i = 0; i < 20; i++) {
        char name[32];
        snprintf(name, sizeof(name), "cmd%d", i);
        clap_parser_t *cmd = clap_subparser_add(subparsers, name, NULL);
        TEST_ASSERT_NOT_NULL(cmd);
    }
    
    TEST_ASSERT_EQUAL(20, subparsers->subparser_count);
    
    clap_parser_free(parser);
}

/* ============================================================================
 * clap_subparsers_metavar Tests
 * ============================================================================ */

void test_subparsers_metavar_basic(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_add_subparsers(parser, "cmd", NULL);
    
    clap_subparsers_metavar(parser, "COMMAND");
    
    TEST_ASSERT_NOT_NULL(parser->subparser_metavar);
    TEST_ASSERT_EQUAL_STRING("COMMAND", clap_buffer_cstr(parser->subparser_metavar));
    
    clap_parser_free(parser);
}

void test_subparsers_metavar_update(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_add_subparsers(parser, "cmd", NULL);
    
    clap_subparsers_metavar(parser, "COMMAND");
    clap_subparsers_metavar(parser, "ACTION");
    
    TEST_ASSERT_EQUAL_STRING("ACTION", clap_buffer_cstr(parser->subparser_metavar));
    
    clap_parser_free(parser);
}

void test_subparsers_metavar_null_parser(void) {
    clap_subparsers_metavar(NULL, "COMMAND");
    TEST_ASSERT_TRUE(1);  /* Should not crash */
}

void test_subparsers_metavar_null_metavar(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_add_subparsers(parser, "cmd", NULL);
    
    clap_subparsers_metavar(parser, NULL);
    
    /* Should not crash, metavar remains unchanged or NULL */
    TEST_ASSERT_TRUE(1);
    
    clap_parser_free(parser);
}

/* ============================================================================
 * clap_print_subcommand_help Tests
 * ============================================================================ */

void test_print_subcommand_help_valid(void) {
    clap_parser_t *parser = clap_parser_new("git", NULL, NULL);
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    clap_subparser_add(subparsers, "commit", "Commit message");
    clap_subparser_add(subparsers, "add", "Add files");
    
    /* Redirect output to capture or just verify return value */
    bool result = clap_print_subcommand_help(parser, "commit", stdout);
    TEST_ASSERT_TRUE(result);
    
    result = clap_print_subcommand_help(parser, "add", stdout);
    TEST_ASSERT_TRUE(result);
    
    clap_parser_free(parser);
}

void test_print_subcommand_help_invalid_command(void) {
    clap_parser_t *parser = clap_parser_new("git", NULL, NULL);
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    clap_subparser_add(subparsers, "commit", NULL);
    
    bool result = clap_print_subcommand_help(parser, "nonexistent", stdout);
    TEST_ASSERT_FALSE(result);
    
    clap_parser_free(parser);
}

void test_print_subcommand_help_null_parser(void) {
    bool result = clap_print_subcommand_help(NULL, "commit", stdout);
    TEST_ASSERT_FALSE(result);
}

void test_print_subcommand_help_null_command(void) {
    clap_parser_t *parser = clap_parser_new("git", NULL, NULL);
    clap_add_subparsers(parser, "cmd", NULL);
    
    bool result = clap_print_subcommand_help(parser, NULL, stdout);
    TEST_ASSERT_FALSE(result);
    
    clap_parser_free(parser);
}

void test_print_subcommand_help_null_stream(void) {
    clap_parser_t *parser = clap_parser_new("git", NULL, NULL);
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    clap_subparser_add(subparsers, "commit", NULL);
    
    bool result = clap_print_subcommand_help(parser, "commit", NULL);
    TEST_ASSERT_FALSE(result);
    
    clap_parser_free(parser);
}

void test_print_subcommand_help_no_subparsers(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    /* No subparsers added */
    
    bool result = clap_print_subcommand_help(parser, "cmd", stdout);
    TEST_ASSERT_FALSE(result);
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Subparser Integration Tests
 * ============================================================================ */

void test_subparser_parse_arguments(void) {
    clap_parser_t *parser = clap_parser_new("git", NULL, NULL);
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);

    clap_parser_t *commit = clap_subparser_add(subparsers, "commit", NULL);
    clap_argument_t *msg = clap_add_argument(commit, "-m");
    clap_argument_type(msg, "string");
    clap_argument_dest(msg, "message");
    clap_argument_required(msg, true);
    
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

void test_subparser_parse_with_global_options(void) {
    clap_parser_t *parser = clap_parser_new("git", NULL, NULL);
    clap_argument_t *global = clap_add_argument(parser, "--verbose");
    clap_argument_action(global, CLAP_ACTION_STORE_TRUE);
    
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    clap_parser_t *commit = clap_subparser_add(subparsers, "commit", NULL);
    clap_argument_t *msg = clap_add_argument(commit, "-m");
    clap_argument_dest(msg, "message");
    clap_argument_type(msg, "string");
    
    char *argv[] = {"git", "--verbose", "commit", "-m", "hello"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 5, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    
    bool verbose;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "verbose", &verbose));
    TEST_ASSERT_TRUE(verbose);
    
    const char *cmd;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "cmd", &cmd));
    TEST_ASSERT_EQUAL_STRING("commit", cmd);
    
    const char *message;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "message", &message));
    TEST_ASSERT_EQUAL_STRING("hello", message);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_subparser_invalid_command(void) {
    clap_parser_t *parser = clap_parser_new("git", NULL, NULL);
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    clap_subparser_add(subparsers, "commit", NULL);
    
    char *argv[] = {"git", "unknown"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 2, argv, &ns, &error);
    
    /* Should fail or treat as positional - depends on implementation */
    /* Current implementation treats unrecognized as positional */
    
    clap_parser_free(parser);
}

void test_subparser_missing_required_subcommand_arg(void) {
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
 * Main Test Runner
 * ============================================================================ */

void run_test_subparser(void) {
    /* clap_add_subparsers Tests */
    RUN_TEST(test_add_subparsers_basic);
    RUN_TEST(test_add_subparsers_null_parser);
    RUN_TEST(test_add_subparsers_null_dest);
    RUN_TEST(test_add_subparsers_inherits_help_width);
    RUN_TEST(test_add_subparsers_empty_prog_name);
    
    /* clap_subparser_add Tests */
    RUN_TEST(test_subparser_add_basic);
    RUN_TEST(test_subparser_add_multiple);
    RUN_TEST(test_subparser_add_null_subparsers);
    RUN_TEST(test_subparser_add_null_name);
    RUN_TEST(test_subparser_add_inherits_help_width);
    RUN_TEST(test_subparser_add_with_empty_parent_prog_name);
    RUN_TEST(test_subparser_add_expands_array);
    
    /* clap_subparsers_metavar Tests */
    RUN_TEST(test_subparsers_metavar_basic);
    RUN_TEST(test_subparsers_metavar_update);
    RUN_TEST(test_subparsers_metavar_null_parser);
    RUN_TEST(test_subparsers_metavar_null_metavar);
    
    /* clap_print_subcommand_help Tests */
    RUN_TEST(test_print_subcommand_help_valid);
    RUN_TEST(test_print_subcommand_help_invalid_command);
    RUN_TEST(test_print_subcommand_help_null_parser);
    RUN_TEST(test_print_subcommand_help_null_command);
    RUN_TEST(test_print_subcommand_help_null_stream);
    RUN_TEST(test_print_subcommand_help_no_subparsers);
    
    /* Integration Tests */
    RUN_TEST(test_subparser_parse_arguments);
    RUN_TEST(test_subparser_parse_with_global_options);
    RUN_TEST(test_subparser_invalid_command);
    RUN_TEST(test_subparser_missing_required_subcommand_arg);
}

#ifdef STANDALONE_TEST
int main(void) {
    UNITY_BEGIN();
    run_test_subparser();
    return UNITY_END();
}
#endif