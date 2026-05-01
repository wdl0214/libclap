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
    
    bool result = clap_parse_args(parser, 7, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    
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
    
    bool result = clap_parse_args(parser, 2, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    
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
    
    bool result = clap_parse_args(parser, 7, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    
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
    
    bool result1 = clap_parse_args(parser, 1, argv1, &ns1, &error1);
    TEST_ASSERT_TRUE(result1);
    
    const char *level_val;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns1, "level", &level_val));
    TEST_ASSERT_EQUAL_STRING("info", level_val);
    
    clap_namespace_free(ns1);
    
    /* Specify --level debug */
    char *argv2[] = {"prog", "--level", "debug"};
    clap_namespace_t *ns2 = NULL;
    clap_error_t error2 = {0};
    
    bool result2 = clap_parse_args(parser, 3, argv2, &ns2, &error2);
    TEST_ASSERT_TRUE(result2);
    
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
    
    bool result = clap_parse_args(parser, 2, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    
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
    
    bool result = clap_parse_args(parser, 3, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    
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
    clap_argument_mutex_group(verbose, group1);
    clap_mutex_group_add_argument(parser, group1, verbose);
    
    clap_argument_t *quiet = clap_add_argument(parser, "--quiet");
    clap_argument_action(quiet, CLAP_ACTION_STORE_TRUE);
    clap_argument_mutex_group(quiet, group1);
    clap_mutex_group_add_argument(parser, group1, quiet);
    
    /* Group 2: format */
    int group2 = clap_add_mutually_exclusive_group(parser, false);
    clap_argument_t *json = clap_add_argument(parser, "--json");
    clap_argument_action(json, CLAP_ACTION_STORE_TRUE);
    clap_argument_mutex_group(json, group2);
    clap_mutex_group_add_argument(parser, group2, json);
    
    clap_argument_t *xml = clap_add_argument(parser, "--xml");
    clap_argument_action(xml, CLAP_ACTION_STORE_TRUE);
    clap_argument_mutex_group(xml, group2);
    clap_mutex_group_add_argument(parser, group2, xml);
    
    /* Valid: one from each group */
    char *argv[] = {"prog", "--verbose", "--json"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 3, argv, &ns, &error);
    TEST_ASSERT_TRUE(result);
    clap_namespace_free(ns);
    
    /* Invalid: conflict in group 1 */
    char *argv2[] = {"prog", "--verbose", "--quiet", "--json"};
    result = clap_parse_args(parser, 4, argv2, &ns, &error);
    TEST_ASSERT_FALSE(result);
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
    
    bool result = clap_parse_args(parser, 5, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    
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
 * Main Test Runner
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
    
    return UNITY_END();
}