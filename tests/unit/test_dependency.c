/**
 * @file test_dependency.c
 * @brief Unit tests for clap_dependency.c
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

static clap_argument_t* create_arg(const char *name) {
    return clap_add_argument(g_parser, name);
}

/* ============================================================================
 * clap_argument_requires Tests
 * ============================================================================ */

void test_argument_requires_basic(void) {
    clap_argument_t *arg1 = create_arg("--input");
    clap_argument_t *arg2 = create_arg("--output");
    
    bool result = clap_argument_requires(arg1, arg2, NULL);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(1, arg1->dependency_count);
    TEST_ASSERT_NOT_NULL(arg1->dependencies);
    TEST_ASSERT_NOT_NULL(arg1->dependencies[0]);
    TEST_ASSERT_EQUAL(CLAP_DEP_REQUIRES, arg1->dependencies[0]->type);
    TEST_ASSERT_EQUAL_PTR(arg1, arg1->dependencies[0]->source);
    TEST_ASSERT_EQUAL(1, arg1->dependencies[0]->target_count);
    TEST_ASSERT_EQUAL_PTR(arg2, arg1->dependencies[0]->targets[0]);
    TEST_ASSERT_NULL(arg1->dependencies[0]->error_message);
}

void test_argument_requires_with_error_message(void) {
    clap_argument_t *arg1 = create_arg("--input");
    clap_argument_t *arg2 = create_arg("--output");
    const char *msg = "Input requires output";
    
    bool result = clap_argument_requires(arg1, arg2, msg);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_STRING(msg, arg1->dependencies[0]->error_message);
}

void test_argument_requires_multiple(void) {
    clap_argument_t *arg1 = create_arg("--input");
    clap_argument_t *arg2 = create_arg("--output");
    clap_argument_t *arg3 = create_arg("--config");
    
    /* Note: requires only supports single target per call */
    TEST_ASSERT_TRUE(clap_argument_requires(arg1, arg2, NULL));
    TEST_ASSERT_TRUE(clap_argument_requires(arg1, arg3, NULL));
    
    TEST_ASSERT_EQUAL(2, arg1->dependency_count);
    TEST_ASSERT_EQUAL_PTR(arg2, arg1->dependencies[0]->targets[0]);
    TEST_ASSERT_EQUAL_PTR(arg3, arg1->dependencies[1]->targets[0]);
}

void test_argument_requires_null_arg(void) {
    clap_argument_t *arg = create_arg("--input");
    
    TEST_ASSERT_FALSE(clap_argument_requires(NULL, arg, NULL));
    TEST_ASSERT_FALSE(clap_argument_requires(arg, NULL, NULL));
    TEST_ASSERT_FALSE(clap_argument_requires(NULL, NULL, NULL));
}

void test_argument_requires_max_dependencies(void) {
    clap_argument_t *arg1 = create_arg("--main");
    
    /* Add up to CLAP_MAX_DEPENDENCIES */
    for (int i = 0; i < CLAP_MAX_DEPENDENCIES; i++) {
        char name[32];
        snprintf(name, sizeof(name), "--dep%d", i);
        clap_argument_t *dep = create_arg(name);
        TEST_ASSERT_TRUE(clap_argument_requires(arg1, dep, NULL));
    }
    
    /* One more should fail */
    clap_argument_t *extra = create_arg("--extra");
    TEST_ASSERT_FALSE(clap_argument_requires(arg1, extra, NULL));
    
    TEST_ASSERT_EQUAL(CLAP_MAX_DEPENDENCIES, arg1->dependency_count);
}

/* ============================================================================
 * clap_argument_conflicts Tests
 * ============================================================================ */

void test_argument_conflicts_basic(void) {
    clap_argument_t *arg1 = create_arg("--verbose");
    clap_argument_t *arg2 = create_arg("--quiet");
    
    bool result = clap_argument_conflicts(arg1, arg2, NULL);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(1, arg1->dependency_count);
    TEST_ASSERT_NOT_NULL(arg1->dependencies);
    TEST_ASSERT_NOT_NULL(arg1->dependencies[0]);
    TEST_ASSERT_EQUAL(CLAP_DEP_CONFLICTS, arg1->dependencies[0]->type);
    TEST_ASSERT_EQUAL_PTR(arg1, arg1->dependencies[0]->source);
    TEST_ASSERT_EQUAL(1, arg1->dependencies[0]->target_count);
    TEST_ASSERT_EQUAL_PTR(arg2, arg1->dependencies[0]->targets[0]);
    TEST_ASSERT_NULL(arg1->dependencies[0]->error_message);
}

void test_argument_conflicts_with_error_message(void) {
    clap_argument_t *arg1 = create_arg("--verbose");
    clap_argument_t *arg2 = create_arg("--quiet");
    const char *msg = "Cannot use verbose and quiet together";
    
    bool result = clap_argument_conflicts(arg1, arg2, msg);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_STRING(msg, arg1->dependencies[0]->error_message);
}

void test_argument_conflicts_multiple(void) {
    clap_argument_t *arg1 = create_arg("--format");
    clap_argument_t *arg2 = create_arg("--json");
    clap_argument_t *arg3 = create_arg("--xml");
    
    TEST_ASSERT_TRUE(clap_argument_conflicts(arg1, arg2, NULL));
    TEST_ASSERT_TRUE(clap_argument_conflicts(arg1, arg3, NULL));
    
    TEST_ASSERT_EQUAL(2, arg1->dependency_count);
    TEST_ASSERT_EQUAL_PTR(arg2, arg1->dependencies[0]->targets[0]);
    TEST_ASSERT_EQUAL_PTR(arg3, arg1->dependencies[1]->targets[0]);
}

void test_argument_conflicts_null_arg(void) {
    clap_argument_t *arg = create_arg("--verbose");
    
    TEST_ASSERT_FALSE(clap_argument_conflicts(NULL, arg, NULL));
    TEST_ASSERT_FALSE(clap_argument_conflicts(arg, NULL, NULL));
    TEST_ASSERT_FALSE(clap_argument_conflicts(NULL, NULL, NULL));
}

void test_argument_conflicts_max_dependencies(void) {
    clap_argument_t *arg1 = create_arg("--main");
    
    for (int i = 0; i < CLAP_MAX_DEPENDENCIES; i++) {
        char name[32];
        snprintf(name, sizeof(name), "--conflict%d", i);
        clap_argument_t *conflict = create_arg(name);
        TEST_ASSERT_TRUE(clap_argument_conflicts(arg1, conflict, NULL));
    }
    
    clap_argument_t *extra = create_arg("--extra");
    TEST_ASSERT_FALSE(clap_argument_conflicts(arg1, extra, NULL));
    
    TEST_ASSERT_EQUAL(CLAP_MAX_DEPENDENCIES, arg1->dependency_count);
}

/* ============================================================================
 * Mixed Dependency Tests
 * ============================================================================ */

void test_argument_mixed_requires_and_conflicts(void) {
    clap_argument_t *arg1 = create_arg("--main");
    clap_argument_t *arg2 = create_arg("--required");
    clap_argument_t *arg3 = create_arg("--conflict");
    
    TEST_ASSERT_TRUE(clap_argument_requires(arg1, arg2, NULL));
    TEST_ASSERT_TRUE(clap_argument_conflicts(arg1, arg3, NULL));
    
    TEST_ASSERT_EQUAL(2, arg1->dependency_count);
    TEST_ASSERT_EQUAL(CLAP_DEP_REQUIRES, arg1->dependencies[0]->type);
    TEST_ASSERT_EQUAL(CLAP_DEP_CONFLICTS, arg1->dependencies[1]->type);
}

void test_argument_symmetric_dependencies(void) {
    /* A conflicts with B, and B also conflicts with A */
    clap_argument_t *arg1 = create_arg("--verbose");
    clap_argument_t *arg2 = create_arg("--quiet");
    
    TEST_ASSERT_TRUE(clap_argument_conflicts(arg1, arg2, NULL));
    TEST_ASSERT_TRUE(clap_argument_conflicts(arg2, arg1, NULL));
    
    TEST_ASSERT_EQUAL(1, arg1->dependency_count);
    TEST_ASSERT_EQUAL(1, arg2->dependency_count);
}

void test_argument_dependency_chain(void) {
    /* A requires B, B requires C */
    clap_argument_t *argA = create_arg("--a");
    clap_argument_t *argB = create_arg("--b");
    clap_argument_t *argC = create_arg("--c");
    
    TEST_ASSERT_TRUE(clap_argument_requires(argA, argB, NULL));
    TEST_ASSERT_TRUE(clap_argument_requires(argB, argC, NULL));
    
    TEST_ASSERT_EQUAL(1, argA->dependency_count);
    TEST_ASSERT_EQUAL_PTR(argB, argA->dependencies[0]->targets[0]);
    
    TEST_ASSERT_EQUAL(1, argB->dependency_count);
    TEST_ASSERT_EQUAL_PTR(argC, argB->dependencies[0]->targets[0]);
}

/* ============================================================================
 * Dependency Structure Validation Tests
 * ============================================================================ */

void test_dependency_structure_allocation(void) {
    clap_argument_t *arg = create_arg("--main");
    clap_argument_t *dep = create_arg("--dep");
    
    clap_argument_requires(arg, dep, "test message");
    
    clap_dependency_t *d = arg->dependencies[0];
    
    TEST_ASSERT_NOT_NULL(d);
    TEST_ASSERT_NOT_NULL(d->targets);
    TEST_ASSERT_NOT_NULL(d->error_message);
    
    /* Verify it's properly allocated and can be freed by parser */
    TEST_ASSERT_TRUE(1);
}

void test_dependency_error_message_copy(void) {
    clap_argument_t *arg = create_arg("--main");
    clap_argument_t *dep = create_arg("--dep");
    const char *msg = "Custom error";
    
    clap_argument_requires(arg, dep, msg);
    
    /* Should be a copy, not the original pointer */
    TEST_ASSERT_NOT_EQUAL((uintptr_t)msg, (uintptr_t)arg->dependencies[0]->error_message);
    TEST_ASSERT_EQUAL_STRING(msg, arg->dependencies[0]->error_message);
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

void run_test_dependency(void) {
    /* clap_argument_requires Tests */
    RUN_TEST(test_argument_requires_basic);
    RUN_TEST(test_argument_requires_with_error_message);
    RUN_TEST(test_argument_requires_multiple);
    RUN_TEST(test_argument_requires_null_arg);
    RUN_TEST(test_argument_requires_max_dependencies);
    
    /* clap_argument_conflicts Tests */
    RUN_TEST(test_argument_conflicts_basic);
    RUN_TEST(test_argument_conflicts_with_error_message);
    RUN_TEST(test_argument_conflicts_multiple);
    RUN_TEST(test_argument_conflicts_null_arg);
    RUN_TEST(test_argument_conflicts_max_dependencies);
    
    /* Mixed Dependency Tests */
    RUN_TEST(test_argument_mixed_requires_and_conflicts);
    RUN_TEST(test_argument_symmetric_dependencies);
    RUN_TEST(test_argument_dependency_chain);
    
    /* Dependency Structure Tests */
    RUN_TEST(test_dependency_structure_allocation);
    RUN_TEST(test_dependency_error_message_copy);
}

#ifdef STANDALONE_TEST
int main(void) {
    UNITY_BEGIN();
    run_test_dependency();
    return UNITY_END();
}
#endif