/**
 * @file test_validator.c
 * @brief Unit tests for clap_validator.c
 */

#include "unity.h"
#include "clap_parser_internal.h"
#include <string.h>

/* ============================================================================
 * Test Setup and Teardown
 * ============================================================================ */

/* Test argument structure for validation tests */
static clap_argument_t *create_test_arg(void) {
    clap_argument_t *arg = clap_calloc(1, sizeof(clap_argument_t));
    arg->nargs = 1;
    arg->group_id = -1;
    arg->action = CLAP_ACTION_STORE;
    arg->type_name = clap_buffer_new("string");
    arg->dest = clap_buffer_new("test");
    return arg;
}

static void free_test_arg(clap_argument_t *arg) {
    if (!arg) return;
    clap_buffer_free(arg->type_name);
    clap_buffer_free(arg->dest);
    for (size_t i = 0; i < arg->choice_count; i++) {
        clap_free(arg->choices[i]);
    }
    clap_free(arg->choices);
    clap_free(arg);
}

void setUp(void) {
    /* Nothing needed */
}

void tearDown(void) {
    /* Nothing needed */
}

/* ============================================================================
 * Choice Validation Tests
 * ============================================================================ */

void test_validate_choice_no_choices(void) {
    clap_argument_t *arg = create_test_arg();
    clap_error_t error = {0};
    
    /* No choices set - should always pass */
    TEST_ASSERT_TRUE(clap_validate_choice(arg, "anything", &error));
    TEST_ASSERT_EQUAL(0, error.code);
    
    free_test_arg(arg);
}

void test_validate_choice_valid(void) {
    clap_argument_t *arg = create_test_arg();
    clap_error_t error = {0};
    
    const char *choices[] = {"red", "green", "blue"};
    arg->choices = clap_calloc(3, sizeof(char*));
    for (int i = 0; i < 3; i++) {
        arg->choices[i] = clap_strdup(choices[i]);
    }
    arg->choice_count = 3;
    
    TEST_ASSERT_TRUE(clap_validate_choice(arg, "green", &error));
    TEST_ASSERT_EQUAL(0, error.code);
    
    free_test_arg(arg);
}

void test_validate_choice_invalid(void) {
    clap_argument_t *arg = create_test_arg();
    clap_error_t error = {0};
    
    const char *choices[] = {"red", "green", "blue"};
    arg->choices = clap_calloc(3, sizeof(char*));
    for (int i = 0; i < 3; i++) {
        arg->choices[i] = clap_strdup(choices[i]);
    }
    arg->choice_count = 3;
    
    TEST_ASSERT_FALSE(clap_validate_choice(arg, "yellow", &error));
    TEST_ASSERT_EQUAL(CLAP_ERR_INVALID_CHOICE, error.code);
    TEST_ASSERT_NOT_NULL(strstr(error.message, "yellow"));
    TEST_ASSERT_NOT_NULL(strstr(error.message, "red"));
    TEST_ASSERT_NOT_NULL(strstr(error.message, "green"));
    TEST_ASSERT_NOT_NULL(strstr(error.message, "blue"));
    
    free_test_arg(arg);
}

void test_validate_choice_first(void) {
    clap_argument_t *arg = create_test_arg();
    clap_error_t error = {0};
    
    const char *choices[] = {"apple", "banana", "cherry"};
    arg->choices = clap_calloc(3, sizeof(char*));
    for (int i = 0; i < 3; i++) {
        arg->choices[i] = clap_strdup(choices[i]);
    }
    arg->choice_count = 3;
    
    TEST_ASSERT_TRUE(clap_validate_choice(arg, "apple", &error));
    
    free_test_arg(arg);
}

void test_validate_choice_last(void) {
    clap_argument_t *arg = create_test_arg();
    clap_error_t error = {0};
    
    const char *choices[] = {"apple", "banana", "cherry"};
    arg->choices = clap_calloc(3, sizeof(char*));
    for (int i = 0; i < 3; i++) {
        arg->choices[i] = clap_strdup(choices[i]);
    }
    arg->choice_count = 3;
    
    TEST_ASSERT_TRUE(clap_validate_choice(arg, "cherry", &error));
    
    free_test_arg(arg);
}

void test_validate_choice_single(void) {
    clap_argument_t *arg = create_test_arg();
    clap_error_t error = {0};
    
    arg->choices = clap_calloc(1, sizeof(char*));
    arg->choices[0] = clap_strdup("only");
    arg->choice_count = 1;
    
    TEST_ASSERT_TRUE(clap_validate_choice(arg, "only", &error));
    TEST_ASSERT_FALSE(clap_validate_choice(arg, "other", &error));
    
    free_test_arg(arg);
}

void test_validate_choice_case_sensitive(void) {
    clap_argument_t *arg = create_test_arg();
    clap_error_t error = {0};
    
    const char *choices[] = {"Red", "Green", "Blue"};
    arg->choices = clap_calloc(3, sizeof(char*));
    for (int i = 0; i < 3; i++) {
        arg->choices[i] = clap_strdup(choices[i]);
    }
    arg->choice_count = 3;
    
    /* Should be case-sensitive */
    TEST_ASSERT_TRUE(clap_validate_choice(arg, "Green", &error));
    TEST_ASSERT_FALSE(clap_validate_choice(arg, "green", &error));
    
    free_test_arg(arg);
}

void test_validate_choice_empty_string(void) {
    clap_argument_t *arg = create_test_arg();
    clap_error_t error = {0};
    
    const char *choices[] = {"", "none", "something"};
    arg->choices = clap_calloc(3, sizeof(char*));
    for (int i = 0; i < 3; i++) {
        arg->choices[i] = clap_strdup(choices[i]);
    }
    arg->choice_count = 3;
    
    TEST_ASSERT_TRUE(clap_validate_choice(arg, "", &error));
    
    free_test_arg(arg);
}

/* ============================================================================
 * Nargs Validation Tests - Default (nargs=1)
 * ============================================================================ */

void test_validate_nargs_default_exact(void) {
    clap_argument_t *arg = create_test_arg();
    arg->nargs = 1;
    clap_error_t error = {0};
    
    TEST_ASSERT_TRUE(clap_validate_nargs(arg, 1, &error));
    
    free_test_arg(arg);
}

void test_validate_nargs_default_zero(void) {
    clap_argument_t *arg = create_test_arg();
    arg->nargs = 1;
    clap_error_t error = {0};
    
    TEST_ASSERT_FALSE(clap_validate_nargs(arg, 0, &error));
    TEST_ASSERT_EQUAL(CLAP_ERR_TOO_FEW_ARGS, error.code);
    
    free_test_arg(arg);
}

void test_validate_nargs_default_two(void) {
    clap_argument_t *arg = create_test_arg();
    arg->nargs = 1;
    clap_error_t error = {0};
    
    TEST_ASSERT_FALSE(clap_validate_nargs(arg, 2, &error));
    TEST_ASSERT_EQUAL(CLAP_ERR_TOO_MANY_ARGS, error.code);
    
    free_test_arg(arg);
}

/* ============================================================================
 * Nargs Validation Tests - ZERO_OR_ONE (?)
 * ============================================================================ */

void test_validate_nargs_question_zero(void) {
    clap_argument_t *arg = create_test_arg();
    arg->nargs = CLAP_NARGS_ZERO_OR_ONE;
    clap_error_t error = {0};
    
    TEST_ASSERT_TRUE(clap_validate_nargs(arg, 0, &error));
    
    free_test_arg(arg);
}

void test_validate_nargs_question_one(void) {
    clap_argument_t *arg = create_test_arg();
    arg->nargs = CLAP_NARGS_ZERO_OR_ONE;
    clap_error_t error = {0};
    
    TEST_ASSERT_TRUE(clap_validate_nargs(arg, 1, &error));
    
    free_test_arg(arg);
}

void test_validate_nargs_question_two(void) {
    clap_argument_t *arg = create_test_arg();
    arg->nargs = CLAP_NARGS_ZERO_OR_ONE;
    clap_error_t error = {0};
    
    TEST_ASSERT_FALSE(clap_validate_nargs(arg, 2, &error));
    TEST_ASSERT_EQUAL(CLAP_ERR_TOO_MANY_ARGS, error.code);
    
    free_test_arg(arg);
}

/* ============================================================================
 * Nargs Validation Tests - ZERO_OR_MORE (*)
 * ============================================================================ */

void test_validate_nargs_star_zero(void) {
    clap_argument_t *arg = create_test_arg();
    arg->nargs = CLAP_NARGS_ZERO_OR_MORE;
    clap_error_t error = {0};
    
    TEST_ASSERT_TRUE(clap_validate_nargs(arg, 0, &error));
    
    free_test_arg(arg);
}

void test_validate_nargs_star_one(void) {
    clap_argument_t *arg = create_test_arg();
    arg->nargs = CLAP_NARGS_ZERO_OR_MORE;
    clap_error_t error = {0};
    
    TEST_ASSERT_TRUE(clap_validate_nargs(arg, 1, &error));
    
    free_test_arg(arg);
}

void test_validate_nargs_star_many(void) {
    clap_argument_t *arg = create_test_arg();
    arg->nargs = CLAP_NARGS_ZERO_OR_MORE;
    clap_error_t error = {0};
    
    TEST_ASSERT_TRUE(clap_validate_nargs(arg, 10, &error));
    
    free_test_arg(arg);
}

/* ============================================================================
 * Nargs Validation Tests - ONE_OR_MORE (+)
 * ============================================================================ */

void test_validate_nargs_plus_zero(void) {
    clap_argument_t *arg = create_test_arg();
    arg->nargs = CLAP_NARGS_ONE_OR_MORE;
    clap_error_t error = {0};
    
    TEST_ASSERT_FALSE(clap_validate_nargs(arg, 0, &error));
    TEST_ASSERT_EQUAL(CLAP_ERR_TOO_FEW_ARGS, error.code);
    
    free_test_arg(arg);
}

void test_validate_nargs_plus_one(void) {
    clap_argument_t *arg = create_test_arg();
    arg->nargs = CLAP_NARGS_ONE_OR_MORE;
    clap_error_t error = {0};
    
    TEST_ASSERT_TRUE(clap_validate_nargs(arg, 1, &error));
    
    free_test_arg(arg);
}

void test_validate_nargs_plus_many(void) {
    clap_argument_t *arg = create_test_arg();
    arg->nargs = CLAP_NARGS_ONE_OR_MORE;
    clap_error_t error = {0};
    
    TEST_ASSERT_TRUE(clap_validate_nargs(arg, 5, &error));
    
    free_test_arg(arg);
}

/* ============================================================================
 * Nargs Validation Tests - REMAINDER (...)
 * ============================================================================ */

void test_validate_nargs_remainder_zero(void) {
    clap_argument_t *arg = create_test_arg();
    arg->nargs = CLAP_NARGS_REMAINDER;
    clap_error_t error = {0};
    
    TEST_ASSERT_TRUE(clap_validate_nargs(arg, 0, &error));
    
    free_test_arg(arg);
}

void test_validate_nargs_remainder_many(void) {
    clap_argument_t *arg = create_test_arg();
    arg->nargs = CLAP_NARGS_REMAINDER;
    clap_error_t error = {0};
    
    TEST_ASSERT_TRUE(clap_validate_nargs(arg, 100, &error));
    
    free_test_arg(arg);
}

/* ============================================================================
 * Nargs Validation Tests - Fixed N
 * ============================================================================ */

void test_validate_nargs_fixed_3_exact(void) {
    clap_argument_t *arg = create_test_arg();
    arg->nargs = 3;
    clap_error_t error = {0};
    
    TEST_ASSERT_TRUE(clap_validate_nargs(arg, 3, &error));
    
    free_test_arg(arg);
}

void test_validate_nargs_fixed_3_too_few(void) {
    clap_argument_t *arg = create_test_arg();
    arg->nargs = 3;
    clap_error_t error = {0};
    
    TEST_ASSERT_FALSE(clap_validate_nargs(arg, 2, &error));
    TEST_ASSERT_EQUAL(CLAP_ERR_TOO_FEW_ARGS, error.code);
    
    free_test_arg(arg);
}

void test_validate_nargs_fixed_3_too_many(void) {
    clap_argument_t *arg = create_test_arg();
    arg->nargs = 3;
    clap_error_t error = {0};
    
    TEST_ASSERT_FALSE(clap_validate_nargs(arg, 4, &error));
    TEST_ASSERT_EQUAL(CLAP_ERR_TOO_MANY_ARGS, error.code);
    
    free_test_arg(arg);
}

void test_validate_nargs_fixed_5(void) {
    clap_argument_t *arg = create_test_arg();
    arg->nargs = 5;
    clap_error_t error = {0};
    
    TEST_ASSERT_TRUE(clap_validate_nargs(arg, 5, &error));
    TEST_ASSERT_FALSE(clap_validate_nargs(arg, 4, &error));
    TEST_ASSERT_FALSE(clap_validate_nargs(arg, 6, &error));
    
    free_test_arg(arg);
}

/* ============================================================================
 * Argument Validate Tests
 * ============================================================================ */

void test_argument_validate_positional_valid(void) {
    clap_argument_t *arg = create_test_arg();
    arg->flags |= CLAP_ARG_POSITIONAL;
    arg->nargs = 1;
    arg->action = CLAP_ACTION_STORE;
    clap_error_t error = {0};
    
    TEST_ASSERT_TRUE(clap_argument_validate(arg, &error));
    
    free_test_arg(arg);
}

void test_argument_validate_positional_invalid_nargs(void) {
    clap_argument_t *arg = create_test_arg();
    arg->flags |= CLAP_ARG_POSITIONAL;
    arg->nargs = 0;  /* Invalid for positional */
    clap_error_t error = {0};
    
    TEST_ASSERT_FALSE(clap_argument_validate(arg, &error));
    TEST_ASSERT_EQUAL(CLAP_ERR_INVALID_ARGUMENT, error.code);
    TEST_ASSERT_NOT_NULL(strstr(error.message, "nargs"));
    
    free_test_arg(arg);
}

void test_argument_validate_positional_negative_nargs(void) {
    clap_argument_t *arg = create_test_arg();
    arg->flags |= CLAP_ARG_POSITIONAL;
    arg->nargs = -1;
    clap_error_t error = {0};
    
    TEST_ASSERT_FALSE(clap_argument_validate(arg, &error));
    TEST_ASSERT_EQUAL(CLAP_ERR_INVALID_ARGUMENT, error.code);
    
    free_test_arg(arg);
}

void test_argument_validate_optional_valid(void) {
    clap_argument_t *arg = create_test_arg();
    arg->flags |= CLAP_ARG_OPTIONAL;
    arg->nargs = 0;  /* Optional can have nargs=0 */
    arg->action = CLAP_ACTION_STORE_TRUE;
    clap_error_t error = {0};
    
    TEST_ASSERT_TRUE(clap_argument_validate(arg, &error));
    
    free_test_arg(arg);
}

void test_argument_validate_sets_action_handler(void) {
    clap_argument_t *arg = create_test_arg();
    arg->action = CLAP_ACTION_STORE;
    clap_error_t error = {0};
    
    TEST_ASSERT_TRUE(clap_argument_validate(arg, &error));
    TEST_ASSERT_NOT_NULL(arg->action_handler);
    
    free_test_arg(arg);
}

void test_argument_validate_sets_action_handler_store_true(void) {
    clap_argument_t *arg = create_test_arg();
    arg->action = CLAP_ACTION_STORE_TRUE;
    clap_error_t error = {0};
    
    TEST_ASSERT_TRUE(clap_argument_validate(arg, &error));
    TEST_ASSERT_NOT_NULL(arg->action_handler);
    
    free_test_arg(arg);
}

void test_argument_validate_sets_action_handler_append(void) {
    clap_argument_t *arg = create_test_arg();
    arg->action = CLAP_ACTION_APPEND;
    clap_error_t error = {0};
    
    TEST_ASSERT_TRUE(clap_argument_validate(arg, &error));
    TEST_ASSERT_NOT_NULL(arg->action_handler);
    
    free_test_arg(arg);
}

void test_argument_validate_sets_action_handler_count(void) {
    clap_argument_t *arg = create_test_arg();
    arg->action = CLAP_ACTION_COUNT;
    clap_error_t error = {0};
    
    TEST_ASSERT_TRUE(clap_argument_validate(arg, &error));
    TEST_ASSERT_NOT_NULL(arg->action_handler);
    
    free_test_arg(arg);
}

void test_argument_validate_custom_action_no_handler_set(void) {
    clap_argument_t *arg = create_test_arg();
    arg->action = CLAP_ACTION_CUSTOM;
    clap_error_t error = {0};
    
    /* Custom action doesn't auto-set handler - that's user's responsibility */
    TEST_ASSERT_TRUE(clap_argument_validate(arg, &error));
    TEST_ASSERT_NULL(arg->action_handler);
    
    free_test_arg(arg);
}

void test_argument_validate_invalid_action(void) {
    clap_argument_t *arg = create_test_arg();
    arg->action = (clap_action_t)999;  /* Invalid action */
    clap_error_t error = {0};
    
    TEST_ASSERT_FALSE(clap_argument_validate(arg, &error));
    TEST_ASSERT_EQUAL(CLAP_ERR_INVALID_ARGUMENT, error.code);
    TEST_ASSERT_NOT_NULL(strstr(error.message, "Unknown action"));
    
    free_test_arg(arg);
}

/* ============================================================================
 * Edge Cases
 * ============================================================================ */

void test_validate_nargs_zero_arguments_for_required(void) {
    clap_argument_t *arg = create_test_arg();
    arg->nargs = 1;
    clap_error_t error = {0};
    
    /* 0 arguments for nargs=1 should fail */
    TEST_ASSERT_FALSE(clap_validate_nargs(arg, 0, &error));
    
    free_test_arg(arg);
}

void test_validate_choice_large_set(void) {
    clap_argument_t *arg = create_test_arg();
    clap_error_t error = {0};
    
    /* Create 100 choices */
    arg->choices = clap_calloc(100, sizeof(char*));
    for (int i = 0; i < 100; i++) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "choice%d", i);
        arg->choices[i] = clap_strdup(buffer);
    }
    arg->choice_count = 100;
    
    TEST_ASSERT_TRUE(clap_validate_choice(arg, "choice50", &error));
    TEST_ASSERT_FALSE(clap_validate_choice(arg, "nonexistent", &error));
    
    free_test_arg(arg);
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

void run_test_validator(void) { 
    /* Choice Validation Tests */
    RUN_TEST(test_validate_choice_no_choices);
    RUN_TEST(test_validate_choice_valid);
    RUN_TEST(test_validate_choice_invalid);
    RUN_TEST(test_validate_choice_first);
    RUN_TEST(test_validate_choice_last);
    RUN_TEST(test_validate_choice_single);
    RUN_TEST(test_validate_choice_case_sensitive);
    RUN_TEST(test_validate_choice_empty_string);
    
    /* Default Nargs Tests */
    RUN_TEST(test_validate_nargs_default_exact);
    RUN_TEST(test_validate_nargs_default_zero);
    RUN_TEST(test_validate_nargs_default_two);
    
    /* Nargs '?' Tests */
    RUN_TEST(test_validate_nargs_question_zero);
    RUN_TEST(test_validate_nargs_question_one);
    RUN_TEST(test_validate_nargs_question_two);
    
    /* Nargs '*' Tests */
    RUN_TEST(test_validate_nargs_star_zero);
    RUN_TEST(test_validate_nargs_star_one);
    RUN_TEST(test_validate_nargs_star_many);
    
    /* Nargs '+' Tests */
    RUN_TEST(test_validate_nargs_plus_zero);
    RUN_TEST(test_validate_nargs_plus_one);
    RUN_TEST(test_validate_nargs_plus_many);
    
    /* Nargs REMAINDER Tests */
    RUN_TEST(test_validate_nargs_remainder_zero);
    RUN_TEST(test_validate_nargs_remainder_many);
    
    /* Fixed N Tests */
    RUN_TEST(test_validate_nargs_fixed_3_exact);
    RUN_TEST(test_validate_nargs_fixed_3_too_few);
    RUN_TEST(test_validate_nargs_fixed_3_too_many);
    RUN_TEST(test_validate_nargs_fixed_5);
    
    /* Argument Validate Tests */
    RUN_TEST(test_argument_validate_positional_valid);
    RUN_TEST(test_argument_validate_positional_invalid_nargs);
    RUN_TEST(test_argument_validate_positional_negative_nargs);
    RUN_TEST(test_argument_validate_optional_valid);
    RUN_TEST(test_argument_validate_sets_action_handler);
    RUN_TEST(test_argument_validate_sets_action_handler_store_true);
    RUN_TEST(test_argument_validate_sets_action_handler_append);
    RUN_TEST(test_argument_validate_sets_action_handler_count);
    RUN_TEST(test_argument_validate_custom_action_no_handler_set);
    RUN_TEST(test_argument_validate_invalid_action);
    
    /* Edge Cases */
    RUN_TEST(test_validate_nargs_zero_arguments_for_required);
    RUN_TEST(test_validate_choice_large_set);
}

#ifdef STANDALONE_TEST
int main(void) {
    UNITY_BEGIN();
    run_test_validator();
    return UNITY_END();
}
#endif