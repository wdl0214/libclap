/**
 * @file test_argument.c
 * @brief Unit tests for clap_argument.c
 */

#include "unity.h"
#include "clap_parser_internal.h"
#include <string.h>

/* ============================================================================
 * Test Setup and Teardown
 * ============================================================================ */

static clap_parser_t *g_parser = NULL;

void setUp(void) {
    g_parser = clap_parser_new("prog", NULL, NULL);
}

void tearDown(void) {
    clap_parser_free(g_parser);
    g_parser = NULL;
}

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

/* ============================================================================
 * clap_add_argument Tests - Positional
 * ============================================================================ */

void test_add_argument_positional_basic(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "input");
    
    TEST_ASSERT_NOT_NULL(arg);
    TEST_ASSERT_TRUE(arg->flags & CLAP_ARG_POSITIONAL);
    TEST_ASSERT_EQUAL(1, arg->option_count);
    TEST_ASSERT_EQUAL_STRING("input", arg->option_strings[0]);
    TEST_ASSERT_EQUAL_STRING("input", clap_buffer_cstr(arg->display_name));
    TEST_ASSERT_EQUAL_STRING("input", clap_buffer_cstr(arg->dest));
    TEST_ASSERT_EQUAL(1, arg->nargs);
    TEST_ASSERT_EQUAL(-1, arg->group_id);
    TEST_ASSERT_EQUAL(CLAP_ACTION_STORE, arg->action);
}

void test_add_argument_positional_multiple(void) {
    clap_argument_t *arg1 = clap_add_argument(g_parser, "input");
    clap_argument_t *arg2 = clap_add_argument(g_parser, "output");
    
    TEST_ASSERT_NOT_NULL(arg1);
    TEST_ASSERT_NOT_NULL(arg2);
    TEST_ASSERT_EQUAL(0, arg1->position);
    TEST_ASSERT_EQUAL(1, arg2->position);
    TEST_ASSERT_EQUAL(2, g_parser->positional_count);
}

/* ============================================================================
 * clap_add_argument Tests - Optional (Short)
 * ============================================================================ */

void test_add_argument_short_option(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "-h");
    
    TEST_ASSERT_NOT_NULL(arg);
    TEST_ASSERT_TRUE(arg->flags & CLAP_ARG_OPTIONAL);
    TEST_ASSERT_EQUAL(1, arg->option_count);
    TEST_ASSERT_EQUAL_STRING("-h", arg->option_strings[0]);
    TEST_ASSERT_EQUAL_STRING("-h", clap_buffer_cstr(arg->display_name));
    TEST_ASSERT_EQUAL_STRING("h", clap_buffer_cstr(arg->dest));
}

void test_add_argument_short_option_uppercase(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "-V");
    
    TEST_ASSERT_NOT_NULL(arg);
    TEST_ASSERT_EQUAL_STRING("-V", arg->option_strings[0]);
    TEST_ASSERT_EQUAL_STRING("V", clap_buffer_cstr(arg->dest));
}

/* ============================================================================
 * clap_add_argument Tests - Optional (Long)
 * ============================================================================ */

void test_add_argument_long_option(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "--help");
    
    TEST_ASSERT_NOT_NULL(arg);
    TEST_ASSERT_TRUE(arg->flags & CLAP_ARG_OPTIONAL);
    TEST_ASSERT_EQUAL(1, arg->option_count);
    TEST_ASSERT_EQUAL_STRING("--help", arg->option_strings[0]);
    TEST_ASSERT_EQUAL_STRING("--help", clap_buffer_cstr(arg->display_name));
    TEST_ASSERT_EQUAL_STRING("help", clap_buffer_cstr(arg->dest));
}

void test_add_argument_long_option_with_dashes(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "--dry-run");
    
    TEST_ASSERT_NOT_NULL(arg);
    TEST_ASSERT_EQUAL_STRING("--dry-run", arg->option_strings[0]);
    TEST_ASSERT_EQUAL_STRING("dry_run", clap_buffer_cstr(arg->dest));
}

/* ============================================================================
 * clap_add_argument Tests - Combined
 * ============================================================================ */

void test_add_argument_combined(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "--help/-h");
    
    TEST_ASSERT_NOT_NULL(arg);
    TEST_ASSERT_EQUAL(2, arg->option_count);
    TEST_ASSERT_EQUAL_STRING("--help", arg->option_strings[0]);
    TEST_ASSERT_EQUAL_STRING("-h", arg->option_strings[1]);
    TEST_ASSERT_EQUAL_STRING("--help", clap_buffer_cstr(arg->display_name));
    TEST_ASSERT_EQUAL_STRING("help", clap_buffer_cstr(arg->dest));
}

void test_add_argument_combined_three(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "--output/-o/-O");
    
    TEST_ASSERT_NOT_NULL(arg);
    TEST_ASSERT_EQUAL(3, arg->option_count);
    TEST_ASSERT_EQUAL_STRING("--output", arg->option_strings[0]);
    TEST_ASSERT_EQUAL_STRING("-o", arg->option_strings[1]);
    TEST_ASSERT_EQUAL_STRING("-O", arg->option_strings[2]);
}

void test_add_argument_combined_longest_as_display(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "-v/--verbose");
    
    TEST_ASSERT_NOT_NULL(arg);
    TEST_ASSERT_EQUAL_STRING("--verbose", clap_buffer_cstr(arg->display_name));
    TEST_ASSERT_EQUAL_STRING("verbose", clap_buffer_cstr(arg->dest));
}

/* ============================================================================
 * clap_add_argument Tests - Edge Cases
 * ============================================================================ */

void test_add_argument_null_parser(void) {
    clap_argument_t *arg = clap_add_argument(NULL, "input");
    TEST_ASSERT_NULL(arg);
}

void test_add_argument_null_name(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, NULL);
    TEST_ASSERT_NULL(arg);
}

void test_add_argument_empty_name(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "");
    TEST_ASSERT_NULL(arg);
}

void test_add_argument_expands_array(void) {
    /* Default capacity is 8, plus default help option = 9 total slots needed */
    for (int i = 0; i < 20; i++) {
        char name[32];
        snprintf(name, sizeof(name), "arg%d", i);
        clap_argument_t *arg = clap_add_argument(g_parser, name);
        TEST_ASSERT_NOT_NULL(arg);
    }
    
    TEST_ASSERT_TRUE(g_parser->arg_capacity >= 21);
}

/* ============================================================================
 * clap_argument_help Tests
 * ============================================================================ */

void test_argument_help_basic(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "--verbose");
    
    clap_argument_t *result = clap_argument_help(arg, "Enable verbose output");
    
    TEST_ASSERT_EQUAL_PTR(arg, result);
    TEST_ASSERT_EQUAL_STRING("Enable verbose output", clap_buffer_cstr(arg->help_text));
}

void test_argument_help_overwrite(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "--verbose");
    clap_argument_help(arg, "First help");
    clap_argument_help(arg, "Second help");
    
    TEST_ASSERT_EQUAL_STRING("Second help", clap_buffer_cstr(arg->help_text));
}

void test_argument_help_null_arg(void) {
    clap_argument_t *result = clap_argument_help(NULL, "help");
    TEST_ASSERT_NULL(result);
}

void test_argument_help_null_text(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "--verbose");
    clap_argument_t *result = clap_argument_help(arg, NULL);
    
    TEST_ASSERT_EQUAL_PTR(arg, result);
    TEST_ASSERT_NULL(arg->help_text);
}

/* ============================================================================
 * clap_argument_type Tests
 * ============================================================================ */

void test_argument_type_basic(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "--count");
    
    clap_argument_t *result = clap_argument_type(arg, "int");
    
    TEST_ASSERT_EQUAL_PTR(arg, result);
    TEST_ASSERT_EQUAL_STRING("int", clap_buffer_cstr(arg->type_name));
}

void test_argument_type_overwrite(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "--value");
    clap_argument_type(arg, "string");
    clap_argument_type(arg, "float");
    
    TEST_ASSERT_EQUAL_STRING("float", clap_buffer_cstr(arg->type_name));
}

/* ============================================================================
 * clap_argument_default Tests
 * ============================================================================ */

void test_argument_default_basic(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "--output");
    
    clap_argument_t *result = clap_argument_default(arg, "stdout");
    
    TEST_ASSERT_EQUAL_PTR(arg, result);
    TEST_ASSERT_EQUAL_STRING("stdout", clap_buffer_cstr(arg->default_string));
}

void test_argument_default_overwrite(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "--output");
    clap_argument_default(arg, "stdout");
    clap_argument_default(arg, "file.txt");
    
    TEST_ASSERT_EQUAL_STRING("file.txt", clap_buffer_cstr(arg->default_string));
}

/* ============================================================================
 * clap_argument_required Tests
 * ============================================================================ */

void test_argument_required_true(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "--host");
    
    clap_argument_t *result = clap_argument_required(arg, true);
    
    TEST_ASSERT_EQUAL_PTR(arg, result);
    TEST_ASSERT_TRUE(arg->flags & CLAP_ARG_REQUIRED);
}

void test_argument_required_false(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "--host");
    clap_argument_required(arg, true);
    clap_argument_required(arg, false);
    
    TEST_ASSERT_FALSE(arg->flags & CLAP_ARG_REQUIRED);
}

/* ============================================================================
 * clap_argument_choices Tests
 * ============================================================================ */

void test_argument_choices_basic(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "--color");
    const char *choices[] = {"red", "green", "blue"};
    
    clap_argument_t *result = clap_argument_choices(arg, choices, 3);
    
    TEST_ASSERT_EQUAL_PTR(arg, result);
    TEST_ASSERT_EQUAL(3, arg->choice_count);
    TEST_ASSERT_EQUAL_STRING("red", arg->choices[0]);
    TEST_ASSERT_EQUAL_STRING("green", arg->choices[1]);
    TEST_ASSERT_EQUAL_STRING("blue", arg->choices[2]);
}

void test_argument_choices_overwrite(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "--color");
    const char *choices1[] = {"red", "green"};
    const char *choices2[] = {"fast", "slow"};
    
    clap_argument_choices(arg, choices1, 2);
    clap_argument_choices(arg, choices2, 2);
    
    TEST_ASSERT_EQUAL(2, arg->choice_count);
    TEST_ASSERT_EQUAL_STRING("fast", arg->choices[0]);
    TEST_ASSERT_EQUAL_STRING("slow", arg->choices[1]);
}

void test_argument_choices_null_arg(void) {
    const char *choices[] = {"a", "b"};
    clap_argument_t *result = clap_argument_choices(NULL, choices, 2);
    TEST_ASSERT_NULL(result);
}

/* ============================================================================
 * clap_argument_const Tests
 * ============================================================================ */

void test_argument_const_basic(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "--mode");
    
    clap_argument_t *result = clap_argument_const(arg, "fast");
    
    TEST_ASSERT_EQUAL_PTR(arg, result);
    TEST_ASSERT_EQUAL_STRING("fast", clap_buffer_cstr(arg->const_value));
}

/* ============================================================================
 * clap_argument_nargs Tests
 * ============================================================================ */

void test_argument_nargs_star(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "files");
    
    clap_argument_t *result = clap_argument_nargs(arg, '*');
    
    TEST_ASSERT_EQUAL_PTR(arg, result);
    TEST_ASSERT_EQUAL(CLAP_NARGS_ZERO_OR_MORE, arg->nargs);
    TEST_ASSERT_TRUE(arg->flags & CLAP_ARG_MULTIPLE);
}

void test_argument_nargs_plus(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "files");
    
    clap_argument_nargs(arg, '+');
    
    TEST_ASSERT_EQUAL(CLAP_NARGS_ONE_OR_MORE, arg->nargs);
    TEST_ASSERT_TRUE(arg->flags & CLAP_ARG_MULTIPLE);
}

void test_argument_nargs_question(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "dest");
    
    clap_argument_nargs(arg, '?');
    
    TEST_ASSERT_EQUAL(CLAP_NARGS_ZERO_OR_ONE, arg->nargs);
    TEST_ASSERT_FALSE(arg->flags & CLAP_ARG_MULTIPLE);
}

void test_argument_nargs_number(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "numbers");
    
    clap_argument_nargs(arg, 3);
    
    TEST_ASSERT_EQUAL(3, arg->nargs);
}

void test_argument_nargs_clear_multiple(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "files");
    clap_argument_nargs(arg, '*');
    TEST_ASSERT_TRUE(arg->flags & CLAP_ARG_MULTIPLE);
    
    clap_argument_nargs(arg, 1);
    TEST_ASSERT_FALSE(arg->flags & CLAP_ARG_MULTIPLE);
}

/* ============================================================================
 * clap_argument_action Tests
 * ============================================================================ */

void test_argument_action_basic(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "--verbose");
    
    clap_argument_t *result = clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    
    TEST_ASSERT_EQUAL_PTR(arg, result);
    TEST_ASSERT_EQUAL(CLAP_ACTION_STORE_TRUE, arg->action);
}

/* ============================================================================
 * clap_argument_metavar Tests
 * ============================================================================ */

void test_argument_metavar_basic(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "--output");
    
    clap_argument_t *result = clap_argument_metavar(arg, "FILE");
    
    TEST_ASSERT_EQUAL_PTR(arg, result);
    TEST_ASSERT_EQUAL_STRING("FILE", clap_buffer_cstr(arg->metavar));
}

/* ============================================================================
 * clap_argument_group Tests
 * ============================================================================ */

void test_argument_group_basic(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "--verbose");
    
    clap_argument_t *result = clap_argument_group(arg, 5);
    
    TEST_ASSERT_EQUAL_PTR(arg, result);
    TEST_ASSERT_EQUAL(5, arg->group_id);
}

/* ============================================================================
 * clap_argument_dest Tests
 * ============================================================================ */

void test_argument_dest_basic(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "--output");
    
    clap_argument_t *result = clap_argument_dest(arg, "out_file");
    
    TEST_ASSERT_EQUAL_PTR(arg, result);
    TEST_ASSERT_EQUAL_STRING("out_file", clap_buffer_cstr(arg->dest));
}

/* ============================================================================
 * clap_argument_handler Tests
 * ============================================================================ */

static bool test_handler(clap_parser_t *p, clap_argument_t *a, clap_namespace_t *n,
                          const char **v, size_t c, void *d, clap_error_t *e) {
    (void)p; (void)a; (void)n; (void)v; (void)c; (void)d; (void)e;
    return true;
}

void test_argument_handler_basic(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "--custom");
    
    clap_argument_t *result = clap_argument_handler(arg, test_handler);
    
    TEST_ASSERT_EQUAL_PTR(arg, result);
    TEST_ASSERT_EQUAL_PTR(test_handler, arg->action_handler);
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

void run_test_argument(void) {    
    /* Positional Tests */
    RUN_TEST(test_add_argument_positional_basic);
    RUN_TEST(test_add_argument_positional_multiple);
    
    /* Short Option Tests */
    RUN_TEST(test_add_argument_short_option);
    RUN_TEST(test_add_argument_short_option_uppercase);
    
    /* Long Option Tests */
    RUN_TEST(test_add_argument_long_option);
    RUN_TEST(test_add_argument_long_option_with_dashes);
    
    /* Combined Tests */
    RUN_TEST(test_add_argument_combined);
    RUN_TEST(test_add_argument_combined_three);
    RUN_TEST(test_add_argument_combined_longest_as_display);
    
    /* Edge Cases */
    RUN_TEST(test_add_argument_null_parser);
    RUN_TEST(test_add_argument_null_name);
    RUN_TEST(test_add_argument_empty_name);
    RUN_TEST(test_add_argument_expands_array);
    
    /* Help Tests */
    RUN_TEST(test_argument_help_basic);
    RUN_TEST(test_argument_help_overwrite);
    RUN_TEST(test_argument_help_null_arg);
    RUN_TEST(test_argument_help_null_text);
    
    /* Type Tests */
    RUN_TEST(test_argument_type_basic);
    RUN_TEST(test_argument_type_overwrite);
    
    /* Default Tests */
    RUN_TEST(test_argument_default_basic);
    RUN_TEST(test_argument_default_overwrite);
    
    /* Required Tests */
    RUN_TEST(test_argument_required_true);
    RUN_TEST(test_argument_required_false);
    
    /* Choices Tests */
    RUN_TEST(test_argument_choices_basic);
    RUN_TEST(test_argument_choices_overwrite);
    RUN_TEST(test_argument_choices_null_arg);
    
    /* Const Tests */
    RUN_TEST(test_argument_const_basic);
    
    /* Nargs Tests */
    RUN_TEST(test_argument_nargs_star);
    RUN_TEST(test_argument_nargs_plus);
    RUN_TEST(test_argument_nargs_question);
    RUN_TEST(test_argument_nargs_number);
    RUN_TEST(test_argument_nargs_clear_multiple);
    
    /* Action Tests */
    RUN_TEST(test_argument_action_basic);
    
    /* Metavar Tests */
    RUN_TEST(test_argument_metavar_basic);
    
    /* Group Tests */
    RUN_TEST(test_argument_group_basic);
    
    /* Dest Tests */
    RUN_TEST(test_argument_dest_basic);
    
    /* Handler Tests */
    RUN_TEST(test_argument_handler_basic);
}

#ifdef STANDALONE_TEST
int main(void) {
    UNITY_BEGIN();
    run_test_argument();
    return UNITY_END();
}
#endif