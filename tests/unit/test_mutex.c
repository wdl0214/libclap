/**
 * @file test_mutex.c
 * @brief Unit tests for clap_mutex.c
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
 * clap_add_mutually_exclusive_group Tests
 * ============================================================================ */

void test_add_mutex_group_basic(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    int group_id = clap_add_mutually_exclusive_group(parser, false);
    
    TEST_ASSERT_GREATER_OR_EQUAL(0, group_id);
    TEST_ASSERT_EQUAL(1, parser->mutex_group_count);
    TEST_ASSERT_NOT_NULL(parser->mutex_groups);
    TEST_ASSERT_NOT_NULL(parser->mutex_groups[0]);
    TEST_ASSERT_EQUAL(group_id, parser->mutex_groups[0]->id);
    TEST_ASSERT_FALSE(parser->mutex_groups[0]->required);
    TEST_ASSERT_EQUAL(4, parser->mutex_groups[0]->arg_capacity);
    TEST_ASSERT_EQUAL(0, parser->mutex_groups[0]->arg_count);
    
    clap_parser_free(parser);
}

void test_add_mutex_group_required(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    int group_id = clap_add_mutually_exclusive_group(parser, true);
    
    TEST_ASSERT_GREATER_OR_EQUAL(0, group_id);
    TEST_ASSERT_TRUE(parser->mutex_groups[0]->required);
    
    clap_parser_free(parser);
}

void test_add_mutex_group_multiple(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    int id1 = clap_add_mutually_exclusive_group(parser, false);
    int id2 = clap_add_mutually_exclusive_group(parser, true);
    int id3 = clap_add_mutually_exclusive_group(parser, false);
    
    TEST_ASSERT_EQUAL(0, id1);
    TEST_ASSERT_EQUAL(1, id2);
    TEST_ASSERT_EQUAL(2, id3);
    TEST_ASSERT_EQUAL(3, parser->mutex_group_count);
    
    clap_parser_free(parser);
}

void test_add_mutex_group_null_parser(void) {
    int result = clap_add_mutually_exclusive_group(NULL, false);
    TEST_ASSERT_EQUAL(-1, result);
}

void test_add_mutex_group_expands_parser_array(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    /* Add enough groups to force expansion */
    for (int i = 0; i < 10; i++) {
        int id = clap_add_mutually_exclusive_group(parser, false);
        TEST_ASSERT_EQUAL(i, id);
    }
    
    TEST_ASSERT_EQUAL(10, parser->mutex_group_count);
    
    clap_parser_free(parser);
}

/* ============================================================================
 * clap_mutex_group_add_argument Tests
 * ============================================================================ */

void test_mutex_group_add_argument_basic(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group_id = clap_add_mutually_exclusive_group(parser, false);
    
    clap_argument_t *arg1 = clap_add_argument(parser, "--verbose");
    clap_argument_action(arg1, CLAP_ACTION_STORE_TRUE);
    
    bool result = clap_mutex_group_add_argument(parser, group_id, arg1);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(1, parser->mutex_groups[0]->arg_count);
    TEST_ASSERT_EQUAL_PTR(arg1, parser->mutex_groups[0]->arguments[0]);
    TEST_ASSERT_EQUAL(group_id, arg1->group_id);
    
    clap_parser_free(parser);
}

void test_mutex_group_add_argument_multiple(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group_id = clap_add_mutually_exclusive_group(parser, false);
    
    clap_argument_t *arg1 = clap_add_argument(parser, "--verbose");
    clap_argument_action(arg1, CLAP_ACTION_STORE_TRUE);
    clap_argument_t *arg2 = clap_add_argument(parser, "--quiet");
    clap_argument_action(arg2, CLAP_ACTION_STORE_TRUE);
    clap_argument_t *arg3 = clap_add_argument(parser, "--silent");
    clap_argument_action(arg3, CLAP_ACTION_STORE_TRUE);
    
    clap_mutex_group_add_argument(parser, group_id, arg1);
    clap_mutex_group_add_argument(parser, group_id, arg2);
    clap_mutex_group_add_argument(parser, group_id, arg3);
    
    TEST_ASSERT_EQUAL(3, parser->mutex_groups[0]->arg_count);
    TEST_ASSERT_EQUAL_PTR(arg1, parser->mutex_groups[0]->arguments[0]);
    TEST_ASSERT_EQUAL_PTR(arg2, parser->mutex_groups[0]->arguments[1]);
    TEST_ASSERT_EQUAL_PTR(arg3, parser->mutex_groups[0]->arguments[2]);
    
    clap_parser_free(parser);
}

void test_mutex_group_add_argument_expands_array(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group_id = clap_add_mutually_exclusive_group(parser, false);
    
    /* Add enough arguments to force expansion (initial capacity is 4) */
    for (int i = 0; i < 10; i++) {
        char name[32];
        snprintf(name, sizeof(name), "--opt%d", i);
        clap_argument_t *arg = clap_add_argument(parser, name);
        clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
        
        bool result = clap_mutex_group_add_argument(parser, group_id, arg);
        TEST_ASSERT_TRUE(result);
    }
    
    TEST_ASSERT_EQUAL(10, parser->mutex_groups[0]->arg_count);
    TEST_ASSERT_TRUE(parser->mutex_groups[0]->arg_capacity >= 10);
    
    clap_parser_free(parser);
}

void test_mutex_group_add_argument_null_parser(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group_id = clap_add_mutually_exclusive_group(parser, false);
    clap_argument_t *arg = clap_add_argument(parser, "--verbose");
    
    bool result = clap_mutex_group_add_argument(NULL, group_id, arg);
    TEST_ASSERT_FALSE(result);
    
    clap_parser_free(parser);
}

void test_mutex_group_add_argument_invalid_group_id(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--verbose");
    
    bool result = clap_mutex_group_add_argument(parser, -1, arg);
    TEST_ASSERT_FALSE(result);
    
    result = clap_mutex_group_add_argument(parser, 999, arg);
    TEST_ASSERT_FALSE(result);
    
    clap_parser_free(parser);
}

void test_mutex_group_add_argument_null_arg(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group_id = clap_add_mutually_exclusive_group(parser, false);
    
    bool result = clap_mutex_group_add_argument(parser, group_id, NULL);
    TEST_ASSERT_FALSE(result);
    
    clap_parser_free(parser);
}

void test_mutex_group_add_argument_sets_group_id(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group_id = clap_add_mutually_exclusive_group(parser, false);
    
    clap_argument_t *arg = clap_add_argument(parser, "--verbose");
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    
    clap_mutex_group_add_argument(parser, group_id, arg);
    
    TEST_ASSERT_EQUAL(group_id, arg->group_id);
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Mutex Group Integration Tests
 * ============================================================================ */

void test_mutex_group_conflict_detection(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group_id = clap_add_mutually_exclusive_group(parser, false);
    
    clap_argument_t *verbose = clap_add_argument(parser, "--verbose");
    clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);
    clap_argument_group(verbose, group_id);
    clap_mutex_group_add_argument(parser, group_id, verbose);
    
    clap_argument_t *quiet = clap_add_argument(parser, "--quiet");
    clap_argument_action(quiet, CLAP_ACTION_STORE_TRUE);
    clap_argument_group(quiet, group_id);
    clap_mutex_group_add_argument(parser, group_id, quiet);
    
    char *argv[] = {"prog", "--verbose", "--quiet"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 3, argv, &ns, &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_MUTUALLY_EXCLUSIVE, error.code);
    
    clap_parser_free(parser);
}

void test_mutex_group_no_conflict_single_option(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group_id = clap_add_mutually_exclusive_group(parser, false);
    
    clap_argument_t *verbose = clap_add_argument(parser, "--verbose");
    clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);
    clap_argument_group(verbose, group_id);
    clap_mutex_group_add_argument(parser, group_id, verbose);
    
    clap_argument_t *quiet = clap_add_argument(parser, "--quiet");
    clap_argument_action(quiet, CLAP_ACTION_STORE_TRUE);
    clap_argument_group(quiet, group_id);
    clap_mutex_group_add_argument(parser, group_id, quiet);
    
    char *argv[] = {"prog", "--verbose"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 2, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    
    bool value;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "verbose", &value));
    TEST_ASSERT_TRUE(value);
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "quiet", &value));
    TEST_ASSERT_FALSE(value);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_mutex_group_required_missing(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group_id = clap_add_mutually_exclusive_group(parser, true);
    
    clap_argument_t *start = clap_add_argument(parser, "--start");
    clap_argument_group(start, group_id);
    clap_mutex_group_add_argument(parser, group_id, start);
    
    clap_argument_t *stop = clap_add_argument(parser, "--stop");
    clap_argument_group(stop, group_id);
    clap_mutex_group_add_argument(parser, group_id, stop);
    
    char *argv[] = {"prog"};  /* Neither --start nor --stop */
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 1, argv, &ns, &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_REQUIRED_MISSING, error.code);
    
    clap_parser_free(parser);
}

void test_mutex_group_required_satisfied(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group_id = clap_add_mutually_exclusive_group(parser, true);
    
    clap_argument_t *start = clap_add_argument(parser, "--start");
    clap_argument_group(start, group_id);
    clap_mutex_group_add_argument(parser, group_id, start);
    
    clap_argument_t *stop = clap_add_argument(parser, "--stop");
    clap_argument_group(stop, group_id);
    clap_mutex_group_add_argument(parser, group_id, stop);
    
    char *argv[] = {"prog", "--start"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 2, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_mutex_group_with_values(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group_id = clap_add_mutually_exclusive_group(parser, false);
    
    clap_argument_t *output = clap_add_argument(parser, "--output");
    clap_argument_type(output, "string");
    clap_argument_group(output, group_id);
    clap_mutex_group_add_argument(parser, group_id, output);
    
    clap_argument_t *stdout_arg = clap_add_argument(parser, "--stdout");
    clap_argument_action(stdout_arg, CLAP_ACTION_STORE_TRUE);
    clap_argument_group(stdout_arg, group_id);
    clap_mutex_group_add_argument(parser, group_id, stdout_arg);
    
    /* Test with --output */
    char *argv1[] = {"prog", "--output", "file.txt"};
    clap_namespace_t *ns1 = NULL;
    clap_error_t error1 = {0};
    
    bool result1 = clap_parse_args(parser, 3, argv1, &ns1, &error1);
    TEST_ASSERT_TRUE(result1);
    
    const char *out_val;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns1, "output", &out_val));
    TEST_ASSERT_EQUAL_STRING("file.txt", out_val);
    
    clap_namespace_free(ns1);
    
    /* Test with --stdout */
    char *argv2[] = {"prog", "--stdout"};
    clap_namespace_t *ns2 = NULL;
    clap_error_t error2 = {0};
    
    bool result2 = clap_parse_args(parser, 2, argv2, &ns2, &error2);
    TEST_ASSERT_TRUE(result2);
    
    bool stdout_val;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns2, "stdout", &stdout_val));
    TEST_ASSERT_TRUE(stdout_val);
    
    clap_namespace_free(ns2);
    
    clap_parser_free(parser);
}

void test_mutex_group_multiple_groups(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    /* Group 1: verbosity */
    int group1 = clap_add_mutually_exclusive_group(parser, false);
    clap_argument_t *verbose = clap_add_argument(parser, "--verbose");
    clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);
    clap_argument_group(verbose, group1);
    clap_mutex_group_add_argument(parser, group1, verbose);
    
    clap_argument_t *quiet = clap_add_argument(parser, "--quiet");
    clap_argument_action(quiet, CLAP_ACTION_STORE_TRUE);
    clap_argument_group(quiet, group1);
    clap_mutex_group_add_argument(parser, group1, quiet);
    
    /* Group 2: format */
    int group2 = clap_add_mutually_exclusive_group(parser, false);
    clap_argument_t *json = clap_add_argument(parser, "--json");
    clap_argument_action(json, CLAP_ACTION_STORE_TRUE);
    clap_argument_group(json, group2);
    clap_mutex_group_add_argument(parser, group2, json);
    
    clap_argument_t *xml = clap_add_argument(parser, "--xml");
    clap_argument_action(xml, CLAP_ACTION_STORE_TRUE);
    clap_argument_group(xml, group2);
    clap_mutex_group_add_argument(parser, group2, xml);
    
    /* Valid: one from each group */
    char *argv[] = {"prog", "--verbose", "--json"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    bool result = clap_parse_args(parser, 3, argv, &ns, &error);
    
    TEST_ASSERT_TRUE(result);
    
    bool v, q, j, x;
    clap_namespace_get_bool(ns, "verbose", &v);
    clap_namespace_get_bool(ns, "quiet", &q);
    clap_namespace_get_bool(ns, "json", &j);
    clap_namespace_get_bool(ns, "xml", &x);
    
    TEST_ASSERT_TRUE(v);
    TEST_ASSERT_FALSE(q);
    TEST_ASSERT_TRUE(j);
    TEST_ASSERT_FALSE(x);
    
    clap_namespace_free(ns);
    clap_parser_free(parser);
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

void run_test_mutex(void) {
    /* clap_add_mutually_exclusive_group Tests */
    RUN_TEST(test_add_mutex_group_basic);
    RUN_TEST(test_add_mutex_group_required);
    RUN_TEST(test_add_mutex_group_multiple);
    RUN_TEST(test_add_mutex_group_null_parser);
    RUN_TEST(test_add_mutex_group_expands_parser_array);
    
    /* clap_mutex_group_add_argument Tests */
    RUN_TEST(test_mutex_group_add_argument_basic);
    RUN_TEST(test_mutex_group_add_argument_multiple);
    RUN_TEST(test_mutex_group_add_argument_expands_array);
    RUN_TEST(test_mutex_group_add_argument_null_parser);
    RUN_TEST(test_mutex_group_add_argument_invalid_group_id);
    RUN_TEST(test_mutex_group_add_argument_null_arg);
    RUN_TEST(test_mutex_group_add_argument_sets_group_id);
    
    /* Integration Tests */
    RUN_TEST(test_mutex_group_conflict_detection);
    RUN_TEST(test_mutex_group_no_conflict_single_option);
    RUN_TEST(test_mutex_group_required_missing);
    RUN_TEST(test_mutex_group_required_satisfied);
    RUN_TEST(test_mutex_group_with_values);
    RUN_TEST(test_mutex_group_multiple_groups);
}

#ifdef STANDALONE_TEST
int main(void) {
    UNITY_BEGIN();
    run_test_mutex();
    return UNITY_END();
}
#endif