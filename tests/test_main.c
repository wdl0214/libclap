/**
 * @file test_main.c
 * @brief Main test runner - runs all unit tests
 */

#include "unity.h"

/* ============================================================================
 * External Test Runners
 * ============================================================================ */

/* Each module should define a run function */
extern void run_test_clap(void);
extern void run_test_allocator(void);
extern void run_test_arena(void);
extern void run_test_buffer(void);
extern void run_test_trie(void);
extern void run_test_error(void);
extern void run_test_parser(void);
extern void run_test_argument(void);
extern void run_test_namespace(void);
extern void run_test_validator(void);
extern void run_test_convert(void);
extern void run_test_actions(void);
extern void run_test_action_executor(void);
extern void run_test_mutex(void);
extern void run_test_display_group(void);
extern void run_test_dependency(void);
extern void run_test_subparser(void);
extern void run_test_find(void);
extern void run_test_formatter(void);
extern void run_test_tokenizer(void);

/* ============================================================================
 * Setup and Teardown
 * ============================================================================ */

void setUp(void) {
    /* Global setup - runs before each test */
}

void tearDown(void) {
    /* Global teardown - runs after each test */
}

/* ============================================================================
 * Main
 * ============================================================================ */

int main(void) {
    UNITY_BEGIN();
    
    /* Core */
    run_test_clap();
    run_test_allocator();
    run_test_arena();
    run_test_buffer();
    run_test_trie();
    
    /* Parser */
    run_test_error();
    run_test_parser();
    run_test_argument();
    run_test_namespace();
    run_test_validator();
    run_test_convert();
    run_test_actions();
    run_test_action_executor();
    
    /* Advanced */
    run_test_mutex();
    run_test_display_group();
    run_test_dependency();
    run_test_subparser();
    run_test_find();
    run_test_formatter();
    run_test_tokenizer();
    
    return UNITY_END();
}