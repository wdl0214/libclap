/**
 * @file test_actions.c
 * @brief Unit tests for clap_actions.c
 */

#include "unity.h"
#include "clap_parser_internal.h"
#include <string.h>

/* ============================================================================
 * Test Setup and Teardown
 * ============================================================================ */

static clap_parser_t *g_parser = NULL;
static clap_namespace_t *g_ns = NULL;

void setUp(void) {
    g_parser = clap_parser_new("prog", NULL, NULL);
    g_ns = clap_namespace_new();
}

void tearDown(void) {
    clap_namespace_free(g_ns);
    clap_parser_free(g_parser);
    g_ns = NULL;
    g_parser = NULL;
}

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

static clap_argument_t* create_arg(const char *name, const char *dest) {
    clap_argument_t *arg = clap_add_argument(g_parser, name);
    if (dest) {
        clap_argument_dest(arg, dest);
    }
    return arg;
}

/* ============================================================================
 * clap_action_store Tests
 * ============================================================================ */

void test_action_store_basic(void) {
    clap_argument_t *arg = create_arg("--output", "output");
    const char *values[] = {"file.txt"};
    clap_error_t error = {0};
    
    bool result = clap_action_store(g_parser, arg, g_ns, values, 1, NULL, &error);
    
    TEST_ASSERT_TRUE(result);
    
    const char *stored;
    TEST_ASSERT_TRUE(clap_namespace_get_string(g_ns, "output", &stored));
    TEST_ASSERT_EQUAL_STRING("file.txt", stored);
}

void test_action_store_zero_count(void) {
    clap_argument_t *arg = create_arg("--output", "output");
    clap_error_t error = {0};
    
    bool result = clap_action_store(g_parser, arg, g_ns, NULL, 0, NULL, &error);
    
    TEST_ASSERT_TRUE(result);
    
    const char *stored;
    TEST_ASSERT_FALSE(clap_namespace_get_string(g_ns, "output", &stored));
}

/* ============================================================================
 * clap_action_store_const Tests
 * ============================================================================ */

void test_action_store_const_basic(void) {
    clap_argument_t *arg = create_arg("--fast", "mode");
    clap_argument_const(arg, "fast");
    clap_error_t error = {0};
    
    bool result = clap_action_store_const(g_parser, arg, g_ns, NULL, 0, NULL, &error);
    
    TEST_ASSERT_TRUE(result);
    
    const char *stored;
    TEST_ASSERT_TRUE(clap_namespace_get_string(g_ns, "mode", &stored));
    TEST_ASSERT_EQUAL_STRING("fast", stored);
}

void test_action_store_const_missing_const(void) {
    clap_argument_t *arg = create_arg("--fast", "mode");
    /* No const value set */
    clap_error_t error = {0};
    
    bool result = clap_action_store_const(g_parser, arg, g_ns, NULL, 0, NULL, &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_INVALID_ARGUMENT, error.code);
    TEST_ASSERT_NOT_NULL(strstr(error.message, "STORE_CONST"));
}

/* ============================================================================
 * clap_action_store_true Tests
 * ============================================================================ */

void test_action_store_true_basic(void) {
    clap_argument_t *arg = create_arg("--verbose", "verbose");
    clap_error_t error = {0};
    
    bool result = clap_action_store_true(g_parser, arg, g_ns, NULL, 0, NULL, &error);
    
    TEST_ASSERT_TRUE(result);
    
    bool stored;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(g_ns, "verbose", &stored));
    TEST_ASSERT_TRUE(stored);
}

/* ============================================================================
 * clap_action_store_false Tests
 * ============================================================================ */

void test_action_store_false_basic(void) {
    clap_argument_t *arg = create_arg("--quiet", "verbose");
    clap_error_t error = {0};
    
    bool result = clap_action_store_false(g_parser, arg, g_ns, NULL, 0, NULL, &error);
    
    TEST_ASSERT_TRUE(result);
    
    bool stored;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(g_ns, "verbose", &stored));
    TEST_ASSERT_FALSE(stored);
}

/* ============================================================================
 * clap_action_append Tests
 * ============================================================================ */

void test_action_append_basic(void) {
    clap_argument_t *arg = create_arg("--tag", "tags");
    const char *values[] = {"important", "urgent"};
    clap_error_t error = {0};
    
    bool result = clap_action_append(g_parser, arg, g_ns, values, 2, NULL, &error);
    
    TEST_ASSERT_TRUE(result);
    
    const char **tags;
    size_t count;
    TEST_ASSERT_TRUE(clap_namespace_get_string_array(g_ns, "tags", &tags, &count));
    TEST_ASSERT_EQUAL(2, count);
    TEST_ASSERT_EQUAL_STRING("important", tags[0]);
    TEST_ASSERT_EQUAL_STRING("urgent", tags[1]);
}

void test_action_append_single(void) {
    clap_argument_t *arg = create_arg("--tag", "tags");
    const char *values[] = {"important"};
    clap_error_t error = {0};
    
    clap_action_append(g_parser, arg, g_ns, values, 1, NULL, &error);
    
    const char **tags;
    size_t count;
    TEST_ASSERT_TRUE(clap_namespace_get_string_array(g_ns, "tags", &tags, &count));
    TEST_ASSERT_EQUAL(1, count);
}

void test_action_append_multiple_calls(void) {
    clap_argument_t *arg = create_arg("--tag", "tags");
    const char *values1[] = {"a"};
    const char *values2[] = {"b", "c"};
    clap_error_t error = {0};
    
    clap_action_append(g_parser, arg, g_ns, values1, 1, NULL, &error);
    clap_action_append(g_parser, arg, g_ns, values2, 2, NULL, &error);
    
    const char **tags;
    size_t count;
    TEST_ASSERT_TRUE(clap_namespace_get_string_array(g_ns, "tags", &tags, &count));
    TEST_ASSERT_EQUAL(3, count);
    TEST_ASSERT_EQUAL_STRING("a", tags[0]);
    TEST_ASSERT_EQUAL_STRING("b", tags[1]);
    TEST_ASSERT_EQUAL_STRING("c", tags[2]);
}

/* ============================================================================
 * clap_action_append_const Tests
 * ============================================================================ */

void test_action_append_const_basic(void) {
    clap_argument_t *arg = create_arg("--add-item", "items");
    clap_argument_const(arg, "item");
    clap_error_t error = {0};
    
    bool result = clap_action_append_const(g_parser, arg, g_ns, NULL, 0, NULL, &error);
    
    TEST_ASSERT_TRUE(result);
    
    const char **items;
    size_t count;
    TEST_ASSERT_TRUE(clap_namespace_get_string_array(g_ns, "items", &items, &count));
    TEST_ASSERT_EQUAL(1, count);
    TEST_ASSERT_EQUAL_STRING("item", items[0]);
}

void test_action_append_const_multiple_calls(void) {
    clap_argument_t *arg = create_arg("--add-item", "items");
    clap_argument_const(arg, "item");
    clap_error_t error = {0};
    
    clap_action_append_const(g_parser, arg, g_ns, NULL, 0, NULL, &error);
    clap_action_append_const(g_parser, arg, g_ns, NULL, 0, NULL, &error);
    clap_action_append_const(g_parser, arg, g_ns, NULL, 0, NULL, &error);
    
    const char **items;
    size_t count;
    TEST_ASSERT_TRUE(clap_namespace_get_string_array(g_ns, "items", &items, &count));
    TEST_ASSERT_EQUAL(3, count);
}

void test_action_append_const_missing_const(void) {
    clap_argument_t *arg = create_arg("--add-item", "items");
    /* No const value set */
    clap_error_t error = {0};
    
    bool result = clap_action_append_const(g_parser, arg, g_ns, NULL, 0, NULL, &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_INVALID_ARGUMENT, error.code);
    TEST_ASSERT_NOT_NULL(strstr(error.message, "APPEND_CONST"));
}

/* ============================================================================
 * clap_action_count Tests
 * ============================================================================ */

void test_action_count_basic(void) {
    clap_argument_t *arg = create_arg("-v", "verbose");
    clap_error_t error = {0};
    
    clap_action_count(g_parser, arg, g_ns, NULL, 0, NULL, &error);
    
    int count;
    TEST_ASSERT_TRUE(clap_namespace_get_int(g_ns, "verbose", &count));
    TEST_ASSERT_EQUAL(1, count);
}

void test_action_count_multiple(void) {
    clap_argument_t *arg = create_arg("-v", "verbose");
    clap_error_t error = {0};
    
    clap_action_count(g_parser, arg, g_ns, NULL, 0, NULL, &error);
    clap_action_count(g_parser, arg, g_ns, NULL, 0, NULL, &error);
    clap_action_count(g_parser, arg, g_ns, NULL, 0, NULL, &error);
    
    int count;
    TEST_ASSERT_TRUE(clap_namespace_get_int(g_ns, "verbose", &count));
    TEST_ASSERT_EQUAL(3, count);
}

/* ============================================================================
 * clap_action_custom Tests
 * ============================================================================ */

static bool custom_handler_called = false;
static const char *custom_value = NULL;
static size_t custom_count = 0;

static bool test_custom_handler(clap_parser_t *p, clap_argument_t *a,
                                 clap_namespace_t *n, const char **v,
                                 size_t c, void *d, clap_error_t *e) {
    (void)p; (void)a; (void)n; (void)d; (void)e;
    custom_handler_called = true;
    custom_value = (c > 0) ? v[0] : NULL;
    custom_count = c;
    return true;
}

void test_action_custom_basic(void) {
    clap_argument_t *arg = create_arg("--custom", "custom");
    arg->action_handler = test_custom_handler;
    const char *values[] = {"test_value"};
    clap_error_t error = {0};
    
    custom_handler_called = false;
    custom_value = NULL;
    custom_count = 0;
    
    bool result = clap_action_custom(g_parser, arg, g_ns, values, 1, NULL, &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(custom_handler_called);
    TEST_ASSERT_EQUAL_STRING("test_value", custom_value);
    TEST_ASSERT_EQUAL(1, custom_count);
}

void test_action_custom_missing_handler(void) {
    clap_argument_t *arg = create_arg("--custom", "custom");
    /* No handler set */
    clap_error_t error = {0};
    
    bool result = clap_action_custom(g_parser, arg, g_ns, NULL, 0, NULL, &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_INVALID_ARGUMENT, error.code);
    TEST_ASSERT_NOT_NULL(strstr(error.message, "CUSTOM"));
}

/* ============================================================================
 * get_action_handler Tests
 * ============================================================================ */

void test_get_action_handler_store(void) {
    clap_action_handler_t handler = get_action_handler(CLAP_ACTION_STORE);
    TEST_ASSERT_EQUAL_PTR(clap_action_store, handler);
}

void test_get_action_handler_store_const(void) {
    clap_action_handler_t handler = get_action_handler(CLAP_ACTION_STORE_CONST);
    TEST_ASSERT_EQUAL_PTR(clap_action_store_const, handler);
}

void test_get_action_handler_store_true(void) {
    clap_action_handler_t handler = get_action_handler(CLAP_ACTION_STORE_TRUE);
    TEST_ASSERT_EQUAL_PTR(clap_action_store_true, handler);
}

void test_get_action_handler_store_false(void) {
    clap_action_handler_t handler = get_action_handler(CLAP_ACTION_STORE_FALSE);
    TEST_ASSERT_EQUAL_PTR(clap_action_store_false, handler);
}

void test_get_action_handler_append(void) {
    clap_action_handler_t handler = get_action_handler(CLAP_ACTION_APPEND);
    TEST_ASSERT_EQUAL_PTR(clap_action_append, handler);
}

void test_get_action_handler_append_const(void) {
    clap_action_handler_t handler = get_action_handler(CLAP_ACTION_APPEND_CONST);
    TEST_ASSERT_EQUAL_PTR(clap_action_append_const, handler);
}

void test_get_action_handler_count(void) {
    clap_action_handler_t handler = get_action_handler(CLAP_ACTION_COUNT);
    TEST_ASSERT_EQUAL_PTR(clap_action_count, handler);
}

void test_get_action_handler_custom(void) {
    clap_action_handler_t handler = get_action_handler(CLAP_ACTION_CUSTOM);
    TEST_ASSERT_EQUAL_PTR(clap_action_custom, handler);
}

void test_get_action_handler_help(void) {
    /* HELP and VERSION are handled separately, not via this dispatcher */
    clap_action_handler_t handler = get_action_handler(CLAP_ACTION_HELP);
    TEST_ASSERT_NULL(handler);
}

void test_get_action_handler_version(void) {
    clap_action_handler_t handler = get_action_handler(CLAP_ACTION_VERSION);
    TEST_ASSERT_NULL(handler);
}

void test_get_action_handler_invalid(void) {
    clap_action_handler_t handler = get_action_handler((clap_action_t)999);
    TEST_ASSERT_NULL(handler);
}

/* ============================================================================
 * Action Integration Tests
 * ============================================================================ */

void test_actions_on_same_dest(void) {
    /* Test multiple actions writing to same dest */
    clap_argument_t *arg1 = create_arg("--verbose", "mode");
    clap_argument_t *arg2 = create_arg("--quiet", "mode");
    clap_argument_t *arg3 = create_arg("--count", "mode");
    
    clap_argument_action(arg1, CLAP_ACTION_STORE_TRUE);
    clap_argument_action(arg2, CLAP_ACTION_STORE_FALSE);
    clap_argument_action(arg3, CLAP_ACTION_COUNT);
    
    clap_error_t error = {0};
    
    /* Execute store_true */
    clap_action_store_true(g_parser, arg1, g_ns, NULL, 0, NULL, &error);
    bool val;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(g_ns, "mode", &val));
    TEST_ASSERT_TRUE(val);
    
    /* Overwrite with store_false */
    clap_action_store_false(g_parser, arg2, g_ns, NULL, 0, NULL, &error);
    TEST_ASSERT_TRUE(clap_namespace_get_bool(g_ns, "mode", &val));
    TEST_ASSERT_FALSE(val);
    
    /* Count - note: count works on int, will conflict with bool */
    clap_action_count(g_parser, arg3, g_ns, NULL, 0, NULL, &error);
    int count;
    TEST_ASSERT_TRUE(clap_namespace_get_int(g_ns, "mode", &count));
    TEST_ASSERT_EQUAL(1, count);
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

void run_test_actions(void) {    
    /* STORE Tests */
    RUN_TEST(test_action_store_basic);
    RUN_TEST(test_action_store_zero_count);
    
    /* STORE_CONST Tests */
    RUN_TEST(test_action_store_const_basic);
    RUN_TEST(test_action_store_const_missing_const);
    
    /* STORE_TRUE Tests */
    RUN_TEST(test_action_store_true_basic);
    
    /* STORE_FALSE Tests */
    RUN_TEST(test_action_store_false_basic);
    
    /* APPEND Tests */
    RUN_TEST(test_action_append_basic);
    RUN_TEST(test_action_append_single);
    RUN_TEST(test_action_append_multiple_calls);
    
    /* APPEND_CONST Tests */
    RUN_TEST(test_action_append_const_basic);
    RUN_TEST(test_action_append_const_multiple_calls);
    RUN_TEST(test_action_append_const_missing_const);
    
    /* COUNT Tests */
    RUN_TEST(test_action_count_basic);
    RUN_TEST(test_action_count_multiple);
    
    /* CUSTOM Tests */
    RUN_TEST(test_action_custom_basic);
    RUN_TEST(test_action_custom_missing_handler);
    
    /* get_action_handler Tests */
    RUN_TEST(test_get_action_handler_store);
    RUN_TEST(test_get_action_handler_store_const);
    RUN_TEST(test_get_action_handler_store_true);
    RUN_TEST(test_get_action_handler_store_false);
    RUN_TEST(test_get_action_handler_append);
    RUN_TEST(test_get_action_handler_append_const);
    RUN_TEST(test_get_action_handler_count);
    RUN_TEST(test_get_action_handler_custom);
    RUN_TEST(test_get_action_handler_help);
    RUN_TEST(test_get_action_handler_version);
    RUN_TEST(test_get_action_handler_invalid);
    
    /* Integration Tests */
    RUN_TEST(test_actions_on_same_dest);
}

#ifdef STANDALONE_TEST
int main(void) {
    UNITY_BEGIN();
    run_test_actions();
    return UNITY_END();
}
#endif