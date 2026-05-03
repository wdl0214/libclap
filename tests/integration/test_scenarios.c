/**
 * @file test_scenarios.c
 * @brief Complex usage scenarios tests
 */

#include "unity.h"
#include <clap/clap.h>
#include <string.h>

/* ============================================================================
 * Test Setup and Teardown
 * ============================================================================ */

void setUp(void) {}
void tearDown(void) {}

/* ============================================================================
 * Scenario 1: Git-style commit
 * ============================================================================ */

void test_scenario_git_commit(void) {
    clap_parser_t *parser = clap_parser_new("git", NULL, NULL);
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    
    clap_parser_t *commit = clap_subparser_add(subparsers, "commit", NULL);
    clap_argument_t *msg = clap_add_argument(commit, "--message/-m");
    clap_argument_type(msg, "string");
    clap_argument_required(msg, true);
    
    clap_argument_t *amend = clap_add_argument(commit, "--amend");
    clap_argument_action(amend, CLAP_ACTION_STORE_TRUE);
    
    clap_argument_t *author = clap_add_argument(commit, "--author");
    clap_argument_type(author, "string");
    
    /* Test full commit command */
    char *argv[] = {"git", "commit", "-m", "Fix bug", "--amend", "--author", "John"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    clap_parse_result_t result = clap_parse_args(parser, 7, argv, &ns, &error);
    
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);
    
    const char *cmd, *message, *author_val;
    bool amend_val;
    
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "cmd", &cmd));
    TEST_ASSERT_EQUAL_STRING("commit", cmd);
    
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "message", &message));
    TEST_ASSERT_EQUAL_STRING("Fix bug", message);
    
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "amend", &amend_val));
    TEST_ASSERT_TRUE(amend_val);
    
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "author", &author_val));
    TEST_ASSERT_EQUAL_STRING("John", author_val);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

/* ============================================================================
 * Scenario 2: Verbosity levels with count
 * ============================================================================ */

void test_scenario_verbosity_count(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    clap_argument_t *verbose = clap_add_argument(parser, "-v");
    clap_argument_action(verbose, CLAP_ACTION_COUNT);
    clap_argument_dest(verbose, "verbose");
    
    /* Test -vvv */
    char *argv[] = {"prog", "-vvv"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    clap_parse_result_t result = clap_parse_args(parser, 2, argv, &ns, &error);
    
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);
    
    int verbose_level;
    TEST_ASSERT_TRUE(clap_namespace_get_int(ns, "verbose", &verbose_level));
    TEST_ASSERT_EQUAL(3, verbose_level);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

/* ============================================================================
 * Scenario 3: Multiple append
 * ============================================================================ */

void test_scenario_append_multiple(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    clap_argument_t *include = clap_add_argument(parser, "-I");
    clap_argument_action(include, CLAP_ACTION_APPEND);
    clap_argument_type(include, "string");
    
    char *argv[] = {"prog", "-I", "dir1", "-I", "dir2", "-I", "dir3"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    clap_parse_result_t result = clap_parse_args(parser, 7, argv, &ns, &error);
    
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);
    
    const char **dirs;
    size_t count;
    TEST_ASSERT_TRUE(clap_namespace_get_string_array(ns, "I", &dirs, &count));
    TEST_ASSERT_EQUAL(3, count);
    TEST_ASSERT_EQUAL_STRING("dir1", dirs[0]);
    TEST_ASSERT_EQUAL_STRING("dir2", dirs[1]);
    TEST_ASSERT_EQUAL_STRING("dir3", dirs[2]);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

/* ============================================================================
 * Scenario 4: Choices with default
 * ============================================================================ */

void test_scenario_choices_with_default(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    clap_argument_t *level = clap_add_argument(parser, "--level");
    clap_argument_type(level, "string");
    clap_argument_choices(level, (const char*[]){"debug", "info", "warn", "error"}, 4);
    clap_argument_default(level, "info");
    
    /* No --level specified - should use default */
    char *argv1[] = {"prog"};
    clap_namespace_t *ns1 = NULL;
    clap_error_t error1 = {0};
    
    clap_parse_result_t result1 = clap_parse_args(parser, 1, argv1, &ns1, &error1);
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result1);
    
    const char *level_val;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns1, "level", &level_val));
    TEST_ASSERT_EQUAL_STRING("info", level_val);
    
    clap_namespace_free(ns1);
    
    /* Specify --level debug */
    char *argv2[] = {"prog", "--level", "debug"};
    clap_namespace_t *ns2 = NULL;
    clap_error_t error2 = {0};
    
    clap_parse_result_t result2 = clap_parse_args(parser, 3, argv2, &ns2, &error2);
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result2);
    
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns2, "level", &level_val));
    TEST_ASSERT_EQUAL_STRING("debug", level_val);
    
    clap_namespace_free(ns2);
    clap_parser_free(parser);
}

/* ============================================================================
 * Scenario 5: Combined short options
 * ============================================================================ */

void test_scenario_combined_short_options(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    clap_argument_t *all = clap_add_argument(parser, "-a");
    clap_argument_action(all, CLAP_ACTION_STORE_TRUE);
    
    clap_argument_t *force = clap_add_argument(parser, "-f");
    clap_argument_action(force, CLAP_ACTION_STORE_TRUE);
    
    clap_argument_t *verbose = clap_add_argument(parser, "-v");
    clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);
    
    char *argv[] = {"prog", "-afv"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    clap_parse_result_t result = clap_parse_args(parser, 2, argv, &ns, &error);
    
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);
    
    bool a_val, f_val, v_val;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "a", &a_val));
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "f", &f_val));
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "v", &v_val));
    
    TEST_ASSERT_TRUE(a_val);
    TEST_ASSERT_TRUE(f_val);
    TEST_ASSERT_TRUE(v_val);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

/* ============================================================================
 * Scenario 6: Long option with equals
 * ============================================================================ */

void test_scenario_long_option_equals(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    clap_argument_t *output = clap_add_argument(parser, "--output");
    clap_argument_type(output, "string");
    
    clap_argument_t *config = clap_add_argument(parser, "--config");
    clap_argument_type(config, "string");
    
    char *argv[] = {"prog", "--output=out.txt", "--config=prod.ini"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    clap_parse_result_t result = clap_parse_args(parser, 3, argv, &ns, &error);
    
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);
    
    const char *output_val, *config_val;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "output", &output_val));
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "config", &config_val));
    TEST_ASSERT_EQUAL_STRING("out.txt", output_val);
    TEST_ASSERT_EQUAL_STRING("prod.ini", config_val);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

/* ============================================================================
 * Scenario 7: Multiple mutex groups
 * ============================================================================ */

void test_scenario_multiple_mutex_groups(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    /* Group 1: verbosity */
    int group1 = clap_add_mutually_exclusive_group(parser, false);
    clap_argument_t *verbose = clap_add_argument(parser, "--verbose");
    clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);
    clap_mutex_group_add_argument(parser, group1, verbose);
    
    clap_argument_t *quiet = clap_add_argument(parser, "--quiet");
    clap_argument_action(quiet, CLAP_ACTION_STORE_TRUE);
    clap_mutex_group_add_argument(parser, group1, quiet);
    
    /* Group 2: format */
    int group2 = clap_add_mutually_exclusive_group(parser, false);
    clap_argument_t *json = clap_add_argument(parser, "--json");
    clap_argument_action(json, CLAP_ACTION_STORE_TRUE);
    clap_mutex_group_add_argument(parser, group2, json);
    
    clap_argument_t *xml = clap_add_argument(parser, "--xml");
    clap_argument_action(xml, CLAP_ACTION_STORE_TRUE);
    clap_mutex_group_add_argument(parser, group2, xml);
    
    /* Valid: one from each group */
    char *argv[] = {"prog", "--verbose", "--json"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    clap_parse_result_t result = clap_parse_args(parser, 3, argv, &ns, &error);
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);
    clap_namespace_free(ns);
    
    /* Invalid: conflict in group 1 */
    char *argv2[] = {"prog", "--verbose", "--quiet", "--json"};
    result = clap_parse_args(parser, 4, argv2, &ns, &error);
    TEST_ASSERT_EQUAL(CLAP_PARSE_ERROR, result);
    TEST_ASSERT_EQUAL(CLAP_ERR_MUTUALLY_EXCLUSIVE, error.code);
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Scenario 8: Subcommand with nargs
 * ============================================================================ */

void test_scenario_subcommand_with_nargs(void) {
    clap_parser_t *parser = clap_parser_new("git", NULL, NULL);
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    
    clap_parser_t *add = clap_subparser_add(subparsers, "add", NULL);
    clap_argument_t *files = clap_add_argument(add, "files");
    clap_argument_nargs(files, '+');
    
    char *argv[] = {"git", "add", "a.txt", "b.txt", "c.txt"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    clap_parse_result_t result = clap_parse_args(parser, 5, argv, &ns, &error);
    
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);
    
    const char *cmd;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "cmd", &cmd));
    TEST_ASSERT_EQUAL_STRING("add", cmd);
    
    const char **file_list;
    size_t count;
    TEST_ASSERT_TRUE(clap_namespace_get_string_array(ns, "files", &file_list, &count));
    TEST_ASSERT_EQUAL(3, count);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

/* ============================================================================
 * Scenario 9: Option should not consume next option as value
 * ============================================================================ */

void test_scenario_option_not_consuming_next_option(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);

    clap_argument_t *output = clap_add_argument(parser, "--output");
    clap_argument_type(output, "string");

    clap_argument_t *verbose = clap_add_argument(parser, "--verbose");
    clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);

    char *argv[] = {"prog", "--output", "--verbose"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};

    clap_parse_result_t result = clap_parse_args(parser, 3, argv, &ns, &error);

    /* Parse must fail: --output needs a value but next token is another option */
    TEST_ASSERT_EQUAL(CLAP_PARSE_ERROR, result);
    TEST_ASSERT_EQUAL(CLAP_ERR_MISSING_VALUE, error.code);

    /* ns must be NULL on parse failure */
    TEST_ASSERT_NULL(ns);

    if (ns) clap_namespace_free(ns);
    clap_parser_free(parser);
}

/* ============================================================================
 * Scenario 10: Option with nargs='?' followed by another option
 *
 * Unlike nargs=1, nargs='?' does NOT require a value. When followed by
 * another option, the next token should NOT be consumed and no error
 * should be raised.
 * ============================================================================ */

void test_scenario_optional_nargs_followed_by_option(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);

    clap_argument_t *output = clap_add_argument(parser, "--output/-o");
    clap_argument_type(output, "string");
    clap_argument_nargs(output, '?');
    clap_argument_default(output, "default.txt");

    clap_argument_t *verbose = clap_add_argument(parser, "--verbose/-v");
    clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);

    /* --output? followed by --verbose: --verbose should NOT be consumed */
    char *argv[] = {"prog", "--output", "--verbose"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};

    clap_parse_result_t result = clap_parse_args(parser, 3, argv, &ns, &error);

    /* Should succeed: nargs='?' allows zero values */
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);
    TEST_ASSERT_EQUAL(CLAP_ERR_NONE, error.code);

    /* --output should use its default value */
    const char *output_val = NULL;
    bool has_output = clap_namespace_get_string(ns, "output", &output_val);
    TEST_ASSERT_TRUE(has_output);
    TEST_ASSERT_EQUAL_STRING("default.txt", output_val);

    /* --verbose must still be parsed as its own flag */
    bool verbose_val = false;
    bool has_verbose = clap_namespace_get_bool(ns, "verbose", &verbose_val);
    TEST_ASSERT_TRUE_MESSAGE(has_verbose,
        "--verbose was incorrectly consumed as --output's value despite nargs='?'");
    TEST_ASSERT_TRUE(verbose_val);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

/* ============================================================================
 * Scenario 11: clap_apply_defaults silently swallows type conversion errors
 * ============================================================================ */

void test_scenario_default_conversion_error_ignored(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);

    /* Optional int with an invalid default string */
    clap_argument_t *port = clap_add_argument(parser, "--port");
    clap_argument_type(port, "int");
    clap_argument_default(port, "not_a_number");

    char *argv[] = {"prog"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};

    clap_parse_result_t result = clap_parse_args(parser, 1, argv, &ns, &error);

    TEST_ASSERT_EQUAL(CLAP_PARSE_ERROR, result);
    TEST_ASSERT_EQUAL(CLAP_ERR_TYPE_CONVERSION, error.code);
    TEST_ASSERT_NULL(ns);

    clap_parser_free(parser);
}

/* ============================================================================
 * Scenario 12: Custom type registry not consulted during parsing
 * ============================================================================ */

static bool g_custom_handler_invoked = false;

static bool test_type_handler(const char *input, void *output,
                               size_t output_size, clap_error_t *error) {
    g_custom_handler_invoked = true;
    (void)input;
    (void)output;
    (void)output_size;
    (void)error;
    /* Simulate what string handler does */
    if (output_size != sizeof(char*)) return false;
    *(const char**)output = input;
    return true;
}

void test_scenario_custom_type_registry_not_used(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);

    /* Register a custom type — stored in parser->type_handlers[] */
    clap_register_type(parser, "test_type", test_type_handler, sizeof(char*));

    clap_argument_t *arg = clap_add_argument(parser, "--name");
    clap_argument_type(arg, "test_type");

    g_custom_handler_invoked = false;

    char *argv[] = {"prog", "--name", "hello"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};

    clap_parse_result_t result = clap_parse_args(parser, 3, argv, &ns, &error);
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);

    TEST_ASSERT_TRUE(g_custom_handler_invoked);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

/* ============================================================================
 * Scenario 13: Float type values should be retrievable via get_float
 * ============================================================================ */

void test_scenario_float_type_storage(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);

    clap_argument_t *pi = clap_add_argument(parser, "--pi");
    clap_argument_type(pi, "float");
    clap_argument_help(pi, "Pi value");

    clap_argument_t *ratio = clap_add_argument(parser, "--ratio");
    clap_argument_type(ratio, "float");

    char *argv[] = {"prog", "--pi", "3.14", "--ratio", "0.618"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};

    clap_parse_result_t result = clap_parse_args(parser, 5, argv, &ns, &error);
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);

    double pi_val = -1.0;
    bool got_float = clap_namespace_get_float(ns, "pi", &pi_val);

    TEST_ASSERT_TRUE(pi_val > 3.13 && pi_val < 3.15);

    double ratio_val = -1.0;
    bool got_ratio = clap_namespace_get_float(ns, "ratio", &ratio_val);
    TEST_ASSERT_TRUE(ratio_val > 0.617 && ratio_val < 0.619);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

/* ============================================================================
 * Scenario 14: Int, float, bool defaults via clap_parse_args
 * ============================================================================ */

void test_scenario_typed_defaults(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);

    clap_argument_t *ratio = clap_add_argument(parser, "--ratio");
    clap_argument_type(ratio, "float");
    clap_argument_default(ratio, "3.14");

    clap_argument_t *count = clap_add_argument(parser, "--count");
    clap_argument_type(count, "int");
    clap_argument_default(count, "42");

    clap_argument_t *flag = clap_add_argument(parser, "--flag");
    clap_argument_type(flag, "bool");
    clap_argument_default(flag, "true");

    char *argv[] = {"prog"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};

    clap_parse_result_t result = clap_parse_args(parser, 1, argv, &ns, &error);
    TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);

    /* Float default should be retrievable via get_float */
    double ratio_val;
    TEST_ASSERT_TRUE(clap_namespace_get_float(ns, "ratio", &ratio_val));
    TEST_ASSERT_TRUE(ratio_val > 3.13 && ratio_val < 3.15);

    /* Int default should be retrievable via get_int */
    int count_val;
    TEST_ASSERT_TRUE(clap_namespace_get_int(ns, "count", &count_val));
    TEST_ASSERT_EQUAL(42, count_val);

    /* Bool default should be retrievable via get_bool */
    bool flag_val;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "flag", &flag_val));
    TEST_ASSERT_TRUE(flag_val);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

/* ============================================================================
 * Scenario 15: Negative numbers consumed as option values
 *
 * Python argparse treats tokens like "-1" as numeric values rather than
 * short options when an option argument is expected.  libclap follows
 * the same convention.
 * ============================================================================ */

void test_scenario_negative_number_as_value(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);

    clap_argument_t *count = clap_add_argument(parser, "--count/-c");
    clap_argument_type(count, "int");

    clap_argument_t *ratio = clap_add_argument(parser, "--ratio/-r");
    clap_argument_type(ratio, "float");

    clap_argument_t *verbose = clap_add_argument(parser, "--verbose/-v");
    clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);

    /* Test 1: -c -5  → -5 consumed as value, then --verbose */
    {
        char *argv[] = {"prog", "-c", "-5", "--verbose"};
        clap_namespace_t *ns = NULL;
        clap_error_t error = {0};

        clap_parse_result_t result = clap_parse_args(parser, 4, argv, &ns, &error);
        TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);

        int count_val;
        TEST_ASSERT_TRUE(clap_namespace_get_int(ns, "count", &count_val));
        TEST_ASSERT_EQUAL(-5, count_val);

        bool verbose_val;
        TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "verbose", &verbose_val));
        TEST_ASSERT_TRUE(verbose_val);

        clap_namespace_free(ns);
    }

    /* Test 2: --ratio -3.14  → negative float consumed as value */
    {
        char *argv[] = {"prog", "--ratio", "-3.14"};
        clap_namespace_t *ns = NULL;
        clap_error_t error = {0};

        clap_parse_result_t result = clap_parse_args(parser, 3, argv, &ns, &error);
        TEST_ASSERT_EQUAL(CLAP_PARSE_SUCCESS, result);

        double ratio_val;
        TEST_ASSERT_TRUE(clap_namespace_get_float(ns, "ratio", &ratio_val));
        TEST_ASSERT_TRUE(ratio_val > -3.15 && ratio_val < -3.13);

        clap_namespace_free(ns);
    }

    /* Test 3: -c --verbose  → --verbose NOT a number, error expected */
    {
        char *argv[] = {"prog", "-c", "--verbose"};
        clap_namespace_t *ns = NULL;
        clap_error_t error = {0};

        clap_parse_result_t result = clap_parse_args(parser, 3, argv, &ns, &error);
        TEST_ASSERT_EQUAL(CLAP_PARSE_ERROR, result);
        TEST_ASSERT_NULL(ns);
    }

    /* Test 4: -c -v  → -v NOT a number, error expected */
    {
        char *argv[] = {"prog", "-c", "-v"};
        clap_namespace_t *ns = NULL;
        clap_error_t error = {0};

        clap_parse_result_t result = clap_parse_args(parser, 3, argv, &ns, &error);
        TEST_ASSERT_EQUAL(CLAP_PARSE_ERROR, result);
        TEST_ASSERT_NULL(ns);
    }

    clap_parser_free(parser);
}

/* ============================================================================
 * ============================================================================ */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_scenario_git_commit);
    RUN_TEST(test_scenario_verbosity_count);
    RUN_TEST(test_scenario_append_multiple);
    RUN_TEST(test_scenario_choices_with_default);
    RUN_TEST(test_scenario_combined_short_options);
    RUN_TEST(test_scenario_long_option_equals);
    RUN_TEST(test_scenario_multiple_mutex_groups);
    RUN_TEST(test_scenario_subcommand_with_nargs);
    RUN_TEST(test_scenario_option_not_consuming_next_option);
    RUN_TEST(test_scenario_optional_nargs_followed_by_option);
    RUN_TEST(test_scenario_float_type_storage);
    RUN_TEST(test_scenario_default_conversion_error_ignored);
    RUN_TEST(test_scenario_custom_type_registry_not_used);
    RUN_TEST(test_scenario_typed_defaults);
    RUN_TEST(test_scenario_negative_number_as_value);

    return UNITY_END();
}