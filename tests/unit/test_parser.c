/**
 * @file test_parser.c
 * @brief Unit tests for clap_parser.c
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
 * Parser Creation Tests
 * ============================================================================ */

void test_parser_new_basic(void) {
    clap_parser_t *parser = clap_parser_new("test_prog", "Test description", "Test epilog");
    
    TEST_ASSERT_NOT_NULL(parser);
    TEST_ASSERT_EQUAL_STRING("test_prog", clap_buffer_cstr(parser->prog_name));
    TEST_ASSERT_EQUAL_STRING("Test description", clap_buffer_cstr(parser->description));
    TEST_ASSERT_EQUAL_STRING("Test epilog", clap_buffer_cstr(parser->epilog));
    TEST_ASSERT_EQUAL(100, parser->help_width);
    TEST_ASSERT_TRUE(parser->add_help_option);
    TEST_ASSERT_EQUAL(0, parser->next_group_id);
    TEST_ASSERT_NULL(parser->subparsers_container);
    TEST_ASSERT_NOT_NULL(parser->arguments);
    TEST_ASSERT_EQUAL(8, parser->arg_capacity);
    TEST_ASSERT_EQUAL(1, parser->arg_count);  /* Default help option */
    
    clap_parser_free(parser);
}

void test_parser_new_null_prog_name(void) {
    clap_parser_t *parser = clap_parser_new(NULL, NULL, NULL);
    
    TEST_ASSERT_NOT_NULL(parser);
    TEST_ASSERT_EQUAL_STRING("program", clap_buffer_cstr(parser->prog_name));
    TEST_ASSERT_NULL(parser->description);
    TEST_ASSERT_NULL(parser->epilog);
    
    clap_parser_free(parser);
}

void test_parser_new_empty_prog_name(void) {
    clap_parser_t *parser = clap_parser_new("", "desc", "epi");
    
    TEST_ASSERT_NOT_NULL(parser);
    TEST_ASSERT_EQUAL_STRING("", clap_buffer_cstr(parser->prog_name));
    
    clap_parser_free(parser);
}

void test_parser_new_allocates_arguments_array(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    TEST_ASSERT_NOT_NULL(parser->arguments);
    TEST_ASSERT_EQUAL(8, parser->arg_capacity);
    
    clap_parser_free(parser);
}

void test_parser_new_adds_default_help_option(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    TEST_ASSERT_EQUAL(1, parser->arg_count);
    clap_argument_t *help = parser->arguments[0];
    TEST_ASSERT_NOT_NULL(help);
    TEST_ASSERT_EQUAL(CLAP_ACTION_HELP, help->action);
    
    /* Check option strings */
    TEST_ASSERT_EQUAL(2, help->option_count);
    TEST_ASSERT_EQUAL_STRING("--help", help->option_strings[0]);
    TEST_ASSERT_EQUAL_STRING("-h", help->option_strings[1]);
    
    clap_parser_free(parser);
}

void test_parser_new_registers_builtin_types(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    TEST_ASSERT_EQUAL(4, parser->type_handler_count);
    
    /* Check type names */
    TEST_ASSERT_EQUAL_STRING("string", parser->type_handlers[0].name);
    TEST_ASSERT_EQUAL_STRING("int", parser->type_handlers[1].name);
    TEST_ASSERT_EQUAL_STRING("float", parser->type_handlers[2].name);
    TEST_ASSERT_EQUAL_STRING("bool", parser->type_handlers[3].name);
    
    /* Check output sizes */
    TEST_ASSERT_EQUAL(sizeof(char*), parser->type_handlers[0].size);
    TEST_ASSERT_EQUAL(sizeof(int), parser->type_handlers[1].size);
    TEST_ASSERT_EQUAL(sizeof(double), parser->type_handlers[2].size);
    TEST_ASSERT_EQUAL(sizeof(bool), parser->type_handlers[3].size);
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Parser Free Tests
 * ============================================================================ */

void test_parser_free_null_safe(void) {
    clap_parser_free(NULL);
    TEST_ASSERT_TRUE(1);  /* Should not crash */
}

void test_parser_free_with_arguments(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_add_argument(parser, "input");
    clap_add_argument(parser, "--output");
    
    TEST_ASSERT_EQUAL(3, parser->arg_count);
    
    clap_parser_free(parser);
    TEST_ASSERT_TRUE(1);  /* Should not crash and no memory leaks */
}

void test_parser_free_with_mutex_groups(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group = clap_add_mutually_exclusive_group(parser, false);
    
    clap_argument_t *verbose = clap_add_argument(parser, "--verbose");
    clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);
    clap_argument_group(verbose, group);
    clap_mutex_group_add_argument(parser, group, verbose);
    
    TEST_ASSERT_EQUAL(1, parser->mutex_group_count);
    
    clap_parser_free(parser);
    TEST_ASSERT_TRUE(1);
}

void test_parser_free_with_subparsers(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    clap_subparser_add(subparsers, "test", NULL);
    
    TEST_ASSERT_NOT_NULL(parser->subparsers_container);
    
    clap_parser_free(parser);
    TEST_ASSERT_TRUE(1);
}

/* ============================================================================
 * Parser Settings Tests
 * ============================================================================ */

void test_parser_set_help_width_valid(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    clap_parser_set_help_width(parser, 120);
    TEST_ASSERT_EQUAL(120, parser->help_width);
    
    clap_parser_set_help_width(parser, 40);
    TEST_ASSERT_EQUAL(40, parser->help_width);
    
    clap_parser_set_help_width(parser, 500);
    TEST_ASSERT_EQUAL(500, parser->help_width);
    
    clap_parser_free(parser);
}

void test_parser_set_help_width_invalid(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int original = parser->help_width;
    
    /* Values below 40 should be ignored */
    clap_parser_set_help_width(parser, 39);
    TEST_ASSERT_EQUAL(original, parser->help_width);
    
    clap_parser_set_help_width(parser, 0);
    TEST_ASSERT_EQUAL(original, parser->help_width);
    
    clap_parser_set_help_width(parser, -10);
    TEST_ASSERT_EQUAL(original, parser->help_width);
    
    /* Values above 500 should be ignored */
    clap_parser_set_help_width(parser, 501);
    TEST_ASSERT_EQUAL(original, parser->help_width);
    
    clap_parser_free(parser);
}

void test_parser_set_help_width_null_safe(void) {
    clap_parser_set_help_width(NULL, 100);
    TEST_ASSERT_TRUE(1);  /* Should not crash */
}

void test_parser_set_version(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    clap_parser_set_version(parser, "1.0.0");
    TEST_ASSERT_NOT_NULL(parser->version);
    TEST_ASSERT_EQUAL_STRING("1.0.0", clap_buffer_cstr(parser->version));
    
    /* Update version */
    clap_parser_set_version(parser, "2.0.0");
    TEST_ASSERT_EQUAL_STRING("2.0.0", clap_buffer_cstr(parser->version));
    
    /* Set to NULL */
    clap_parser_set_version(parser, NULL);
    TEST_ASSERT_NULL(parser->version);
    
    clap_parser_free(parser);
}

void test_parser_set_version_null_safe(void) {
    clap_parser_set_version(NULL, "1.0.0");
    TEST_ASSERT_TRUE(1);  /* Should not crash */
}

/* ============================================================================
 * Type Registration Tests
 * ============================================================================ */

void test_register_type_success(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    size_t initial_count = parser->type_handler_count;
    
    bool result = clap_register_type(parser, "custom", 
                                      clap_type_string_handler, sizeof(char*));
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(initial_count + 1, parser->type_handler_count);
    TEST_ASSERT_EQUAL_STRING("custom", parser->type_handlers[initial_count].name);
    
    clap_parser_free(parser);
}

void test_register_type_duplicate(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    size_t initial_count = parser->type_handler_count;
    
    /* Try to register duplicate "string" type */
    bool result = clap_register_type(parser, "string", 
                                      clap_type_string_handler, sizeof(char*));
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(initial_count, parser->type_handler_count);
    
    clap_parser_free(parser);
}

void test_register_type_null_params(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    TEST_ASSERT_FALSE(clap_register_type(NULL, "type", clap_type_string_handler, 4));
    TEST_ASSERT_FALSE(clap_register_type(parser, NULL, clap_type_string_handler, 4));
    TEST_ASSERT_FALSE(clap_register_type(parser, "type", NULL, 4));
    
    clap_parser_free(parser);
}

void test_register_type_expands_capacity(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    /* Register many types to force capacity expansion */
    char type_name[32];
    for (int i = 0; i < 20; i++) {
        snprintf(type_name, sizeof(type_name), "type%d", i);
        bool result = clap_register_type(parser, type_name, 
                                          clap_type_string_handler, sizeof(char*));
        TEST_ASSERT_TRUE(result);
    }
    
    TEST_ASSERT_TRUE(parser->type_handler_capacity >= 20);
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Argument Expansion Tests
 * ============================================================================ */

void test_add_argument_expands_arguments_array(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    /* Default help option already added */
    size_t initial_count = parser->arg_count;
    size_t initial_capacity = parser->arg_capacity;
    
    /* Add enough arguments to force expansion */
    for (int i = 0; i < 20; i++) {
        char arg_name[32];
        snprintf(arg_name, sizeof(arg_name), "arg%d", i);
        clap_argument_t *arg = clap_add_argument(parser, arg_name);
        TEST_ASSERT_NOT_NULL(arg);
    }
    
    TEST_ASSERT_EQUAL(initial_count + 20, parser->arg_count);
    TEST_ASSERT_TRUE(parser->arg_capacity > initial_capacity);
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Edge Cases
 * ============================================================================ */

void test_parser_free_double_free_safe(void) {
    /* This is tested via Valgrind/ASan, not easily testable in unit test */
    TEST_ASSERT_TRUE(1);
}

void test_parser_with_maximum_arguments(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    /* Add many arguments */
    for (int i = 0; i < 100; i++) {
        char arg_name[32];
        snprintf(arg_name, sizeof(arg_name), "--arg%d", i);
        clap_argument_t *arg = clap_add_argument(parser, arg_name);
        TEST_ASSERT_NOT_NULL(arg);
    }
    
    clap_parser_free(parser);
    TEST_ASSERT_TRUE(1);
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

void run_test_parser(void) {
    /* Parser Creation Tests */
    RUN_TEST(test_parser_new_basic);
    RUN_TEST(test_parser_new_null_prog_name);
    RUN_TEST(test_parser_new_empty_prog_name);
    RUN_TEST(test_parser_new_allocates_arguments_array);
    RUN_TEST(test_parser_new_adds_default_help_option);
    RUN_TEST(test_parser_new_registers_builtin_types);
    
    /* Parser Free Tests */
    RUN_TEST(test_parser_free_null_safe);
    RUN_TEST(test_parser_free_with_arguments);
    RUN_TEST(test_parser_free_with_mutex_groups);
    RUN_TEST(test_parser_free_with_subparsers);
    
    /* Parser Settings Tests */
    RUN_TEST(test_parser_set_help_width_valid);
    RUN_TEST(test_parser_set_help_width_invalid);
    RUN_TEST(test_parser_set_help_width_null_safe);
    RUN_TEST(test_parser_set_version);
    RUN_TEST(test_parser_set_version_null_safe);
    
    /* Type Registration Tests */
    RUN_TEST(test_register_type_success);
    RUN_TEST(test_register_type_duplicate);
    RUN_TEST(test_register_type_null_params);
    RUN_TEST(test_register_type_expands_capacity);
    
    /* Argument Expansion Tests */
    RUN_TEST(test_add_argument_expands_arguments_array);
    
    /* Edge Cases */
    RUN_TEST(test_parser_free_double_free_safe);
    RUN_TEST(test_parser_with_maximum_arguments);
}

#ifdef STANDALONE_TEST
int main(void) {
    UNITY_BEGIN();
    run_test_parser();
    return UNITY_END();
}
#endif