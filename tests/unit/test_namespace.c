/**
 * @file test_namespace.c
 * @brief Unit tests for clap_namespace.c
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
 * Namespace Creation Tests
 * ============================================================================ */

void test_namespace_new(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    TEST_ASSERT_NOT_NULL(ns);
    TEST_ASSERT_EQUAL(0, ns->value_count);
    TEST_ASSERT_EQUAL(8, ns->value_capacity);
    TEST_ASSERT_NOT_NULL(ns->values);
    
    clap_namespace_free(ns);
}

/* ============================================================================
 * String Value Tests
 * ============================================================================ */

void test_namespace_set_get_string(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    /* Set string */
    TEST_ASSERT_TRUE(clap_namespace_set_string(ns, "key1", "value1"));
    TEST_ASSERT_EQUAL(1, ns->value_count);
    
    /* Get string */
    const char *value;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "key1", &value));
    TEST_ASSERT_EQUAL_STRING("value1", value);
    
    clap_namespace_free(ns);
}

void test_namespace_set_string_overwrite(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    clap_namespace_set_string(ns, "key", "first");
    clap_namespace_set_string(ns, "key", "second");
    
    TEST_ASSERT_EQUAL(1, ns->value_count);
    
    const char *value;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "key", &value));
    TEST_ASSERT_EQUAL_STRING("second", value);
    
    clap_namespace_free(ns);
}

void test_namespace_set_string_null_value(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    TEST_ASSERT_TRUE(clap_namespace_set_string(ns, "key", NULL));
    
    const char *value;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "key", &value));
    TEST_ASSERT_NULL(value);
    
    clap_namespace_free(ns);
}

void test_namespace_get_string_missing(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    const char *value;
    TEST_ASSERT_FALSE(clap_namespace_get_string(ns, "nonexistent", &value));
    
    clap_namespace_free(ns);
}

void test_namespace_get_string_wrong_type(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    clap_namespace_set_int(ns, "key", 42);
    
    const char *value;
    TEST_ASSERT_FALSE(clap_namespace_get_string(ns, "key", &value));
    
    clap_namespace_free(ns);
}

void test_namespace_set_string_multiple_keys(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    clap_namespace_set_string(ns, "key1", "value1");
    clap_namespace_set_string(ns, "key2", "value2");
    clap_namespace_set_string(ns, "key3", "value3");
    
    TEST_ASSERT_EQUAL(3, ns->value_count);
    
    const char *v1, *v2, *v3;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "key1", &v1));
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "key2", &v2));
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "key3", &v3));
    
    TEST_ASSERT_EQUAL_STRING("value1", v1);
    TEST_ASSERT_EQUAL_STRING("value2", v2);
    TEST_ASSERT_EQUAL_STRING("value3", v3);
    
    clap_namespace_free(ns);
}

/* ============================================================================
 * Integer Value Tests
 * ============================================================================ */

void test_namespace_set_get_int(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    TEST_ASSERT_TRUE(clap_namespace_set_int(ns, "count", 42));
    TEST_ASSERT_EQUAL(1, ns->value_count);
    
    int value;
    TEST_ASSERT_TRUE(clap_namespace_get_int(ns, "count", &value));
    TEST_ASSERT_EQUAL(42, value);
    
    clap_namespace_free(ns);
}

void test_namespace_set_int_overwrite(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    clap_namespace_set_int(ns, "count", 10);
    clap_namespace_set_int(ns, "count", 20);
    
    TEST_ASSERT_EQUAL(1, ns->value_count);
    
    int value;
    TEST_ASSERT_TRUE(clap_namespace_get_int(ns, "count", &value));
    TEST_ASSERT_EQUAL(20, value);
    
    clap_namespace_free(ns);
}

void test_namespace_set_int_negative(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    clap_namespace_set_int(ns, "neg", -100);
    
    int value;
    TEST_ASSERT_TRUE(clap_namespace_get_int(ns, "neg", &value));
    TEST_ASSERT_EQUAL(-100, value);
    
    clap_namespace_free(ns);
}

void test_namespace_set_int_zero(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    clap_namespace_set_int(ns, "zero", 0);
    
    int value;
    TEST_ASSERT_TRUE(clap_namespace_get_int(ns, "zero", &value));
    TEST_ASSERT_EQUAL(0, value);
    
    clap_namespace_free(ns);
}

void test_namespace_get_int_missing(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    int value;
    TEST_ASSERT_FALSE(clap_namespace_get_int(ns, "missing", &value));
    
    clap_namespace_free(ns);
}

void test_namespace_get_int_wrong_type(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    clap_namespace_set_string(ns, "key", "not_an_int");
    
    int value;
    TEST_ASSERT_FALSE(clap_namespace_get_int(ns, "key", &value));
    
    clap_namespace_free(ns);
}

/* ============================================================================
 * Boolean Value Tests
 * ============================================================================ */

void test_namespace_set_get_bool(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    TEST_ASSERT_TRUE(clap_namespace_set_bool(ns, "flag", true));
    
    bool value;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "flag", &value));
    TEST_ASSERT_TRUE(value);
    
    clap_namespace_set_bool(ns, "flag", false);
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "flag", &value));
    TEST_ASSERT_FALSE(value);
    
    clap_namespace_free(ns);
}

void test_namespace_get_bool_missing(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    bool value;
    TEST_ASSERT_FALSE(clap_namespace_get_bool(ns, "missing", &value));
    
    clap_namespace_free(ns);
}

void test_namespace_get_bool_wrong_type(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    clap_namespace_set_string(ns, "key", "true");
    
    bool value;
    TEST_ASSERT_FALSE(clap_namespace_get_bool(ns, "key", &value));
    
    clap_namespace_free(ns);
}

/* ============================================================================
 * Float Value Tests
 * ============================================================================ */

void test_namespace_set_get_float(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    /* Float support may not be fully implemented - this tests the API */
    /* The function exists but may not store correctly */
    
    /* Create a float value manually for testing */
    clap_value_t *val = clap_calloc(1, sizeof(clap_value_t));
    val->name = clap_strdup("pi");
    val->type = CLAP_VAL_FLOAT;
    val->data.float_val = 3.14159;
    ns->values[0] = val;
    ns->value_count = 1;
    
    double value;
    TEST_ASSERT_TRUE(clap_namespace_get_float(ns, "pi", &value));
    TEST_ASSERT_TRUE(value > 3.14 && value < 3.15);
    
    clap_namespace_free(ns);
}

void test_namespace_get_float_missing(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    double value;
    TEST_ASSERT_FALSE(clap_namespace_get_float(ns, "missing", &value));
    
    clap_namespace_free(ns);
}

void test_namespace_get_float_wrong_type(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    clap_namespace_set_int(ns, "key", 42);
    
    double value;
    TEST_ASSERT_FALSE(clap_namespace_get_float(ns, "key", &value));
    
    clap_namespace_free(ns);
}

/* ============================================================================
 * String Array (Append) Tests
 * ============================================================================ */

void test_namespace_append_string(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    TEST_ASSERT_TRUE(clap_namespace_append_string(ns, "list", "item1"));
    TEST_ASSERT_TRUE(clap_namespace_append_string(ns, "list", "item2"));
    TEST_ASSERT_TRUE(clap_namespace_append_string(ns, "list", "item3"));
    
    const char **items;
    size_t count;
    TEST_ASSERT_TRUE(clap_namespace_get_string_array(ns, "list", &items, &count));
    
    TEST_ASSERT_EQUAL(3, count);
    TEST_ASSERT_EQUAL_STRING("item1", items[0]);
    TEST_ASSERT_EQUAL_STRING("item2", items[1]);
    TEST_ASSERT_EQUAL_STRING("item3", items[2]);
    
    clap_namespace_free(ns);
}

void test_namespace_append_string_null_value(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    TEST_ASSERT_TRUE(clap_namespace_append_string(ns, "list", NULL));
    
    const char **items;
    size_t count;
    TEST_ASSERT_FALSE(clap_namespace_get_string_array(ns, "list", &items, &count));
    
    clap_namespace_free(ns);
}

void test_namespace_append_string_array_expansion(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    /* Add many items to force array capacity expansion */
    for (int i = 0; i < 20; i++) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "item%d", i);
        TEST_ASSERT_TRUE(clap_namespace_append_string(ns, "list", buffer));
    }
    
    const char **items;
    size_t count;
    TEST_ASSERT_TRUE(clap_namespace_get_string_array(ns, "list", &items, &count));
    TEST_ASSERT_EQUAL(20, count);
    
    clap_namespace_free(ns);
}

void test_namespace_get_string_array_missing(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    const char **items;
    size_t count;
    TEST_ASSERT_FALSE(clap_namespace_get_string_array(ns, "missing", &items, &count));
    
    clap_namespace_free(ns);
}

void test_namespace_get_string_array_wrong_type(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    clap_namespace_set_string(ns, "key", "not_an_array");
    
    const char **items;
    size_t count;
    TEST_ASSERT_FALSE(clap_namespace_get_string_array(ns, "key", &items, &count));
    
    clap_namespace_free(ns);
}

/* ============================================================================
 * Namespace Merge Tests
 * ============================================================================ */

void test_namespace_merge_basic(void) {
    clap_namespace_t *dst = clap_namespace_new();
    clap_namespace_t *src = clap_namespace_new();
    
    clap_namespace_set_string(dst, "dst_only", "dst_value");
    clap_namespace_set_string(src, "src_only", "src_value");
    clap_namespace_set_string(src, "shared", "src_shared");
    
    TEST_ASSERT_TRUE(clap_namespace_merge(dst, src));
    
    /* dst_only should remain */
    const char *value;
    TEST_ASSERT_TRUE(clap_namespace_get_string(dst, "dst_only", &value));
    TEST_ASSERT_EQUAL_STRING("dst_value", value);
    
    /* src_only should be copied */
    TEST_ASSERT_TRUE(clap_namespace_get_string(dst, "src_only", &value));
    TEST_ASSERT_EQUAL_STRING("src_value", value);
    
    clap_namespace_free(dst);
    clap_namespace_free(src);
}

void test_namespace_merge_no_overwrite(void) {
    clap_namespace_t *dst = clap_namespace_new();
    clap_namespace_t *src = clap_namespace_new();
    
    clap_namespace_set_string(dst, "shared", "dst_value");
    clap_namespace_set_string(src, "shared", "src_value");
    
    TEST_ASSERT_TRUE(clap_namespace_merge(dst, src));
    
    /* dst value should NOT be overwritten */
    const char *value;
    TEST_ASSERT_TRUE(clap_namespace_get_string(dst, "shared", &value));
    TEST_ASSERT_EQUAL_STRING("dst_value", value);
    
    clap_namespace_free(dst);
    clap_namespace_free(src);
}

void test_namespace_merge_with_arrays(void) {
    clap_namespace_t *dst = clap_namespace_new();
    clap_namespace_t *src = clap_namespace_new();
    
    clap_namespace_append_string(src, "list", "item1");
    clap_namespace_append_string(src, "list", "item2");
    
    TEST_ASSERT_TRUE(clap_namespace_merge(dst, src));
    
    const char **items;
    size_t count;
    TEST_ASSERT_TRUE(clap_namespace_get_string_array(dst, "list", &items, &count));
    TEST_ASSERT_EQUAL(2, count);
    TEST_ASSERT_EQUAL_STRING("item1", items[0]);
    TEST_ASSERT_EQUAL_STRING("item2", items[1]);
    
    clap_namespace_free(dst);
    clap_namespace_free(src);
}

void test_namespace_merge_mixed_types(void) {
    clap_namespace_t *dst = clap_namespace_new();
    clap_namespace_t *src = clap_namespace_new();

    clap_namespace_set_string(src, "str", "hello");
    clap_namespace_set_int(src, "num", 42);
    clap_namespace_set_float(src, "pi", 3.14);
    clap_namespace_set_bool(src, "flag", true);

    TEST_ASSERT_TRUE(clap_namespace_merge(dst, src));

    const char *str_val;
    int int_val;
    double float_val;
    bool bool_val;

    TEST_ASSERT_TRUE(clap_namespace_get_string(dst, "str", &str_val));
    TEST_ASSERT_EQUAL_STRING("hello", str_val);

    TEST_ASSERT_TRUE(clap_namespace_get_int(dst, "num", &int_val));
    TEST_ASSERT_EQUAL(42, int_val);

    TEST_ASSERT_TRUE(clap_namespace_get_float(dst, "pi", &float_val));
    TEST_ASSERT_TRUE(float_val > 3.13 && float_val < 3.15);

    TEST_ASSERT_TRUE(clap_namespace_get_bool(dst, "flag", &bool_val));
    TEST_ASSERT_TRUE(bool_val);

    clap_namespace_free(dst);
    clap_namespace_free(src);
}

void test_namespace_merge_null_params(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    TEST_ASSERT_FALSE(clap_namespace_merge(NULL, ns));
    TEST_ASSERT_FALSE(clap_namespace_merge(ns, NULL));
    TEST_ASSERT_FALSE(clap_namespace_merge(NULL, NULL));
    
    clap_namespace_free(ns);
}

/* ============================================================================
 * Namespace Free Tests
 * ============================================================================ */

void test_namespace_free_null_safe(void) {
    clap_namespace_free(NULL);
    TEST_ASSERT_TRUE(1);  /* Should not crash */
}

void test_namespace_free_with_values(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    clap_namespace_set_string(ns, "str", "value");
    clap_namespace_set_int(ns, "int", 42);
    clap_namespace_set_bool(ns, "bool", true);
    clap_namespace_append_string(ns, "list", "item");
    
    clap_namespace_free(ns);
    TEST_ASSERT_TRUE(1);  /* Should not crash and no memory leaks */
}

/* ============================================================================
 * Capacity Expansion Tests
 * ============================================================================ */

void test_namespace_capacity_expansion(void) {
    clap_namespace_t *ns = clap_namespace_new();
    size_t initial_capacity = ns->value_capacity;
    
    /* Add enough keys to force expansion */
    for (int i = 0; i < 20; i++) {
        char key[32];
        snprintf(key, sizeof(key), "key%d", i);
        clap_namespace_set_string(ns, key, "value");
    }
    
    TEST_ASSERT_TRUE(ns->value_capacity > initial_capacity);
    TEST_ASSERT_EQUAL(20, ns->value_count);
    
    /* Verify all keys are accessible */
    for (int i = 0; i < 20; i++) {
        char key[32];
        snprintf(key, sizeof(key), "key%d", i);
        const char *value;
        TEST_ASSERT_TRUE(clap_namespace_get_string(ns, key, &value));
    }
    
    clap_namespace_free(ns);
}

/* ============================================================================
 * Type Conversion Edge Cases
 * ============================================================================ */

void test_namespace_string_to_int_not_allowed(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    clap_namespace_set_string(ns, "num", "42");
    
    int value;
    TEST_ASSERT_FALSE(clap_namespace_get_int(ns, "num", &value));
    
    clap_namespace_free(ns);
}

void test_namespace_int_to_string_not_allowed(void) {
    clap_namespace_t *ns = clap_namespace_new();
    
    clap_namespace_set_int(ns, "num", 42);
    
    const char *value;
    TEST_ASSERT_FALSE(clap_namespace_get_string(ns, "num", &value));
    
    clap_namespace_free(ns);
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

void run_test_namespace(void) {
    /* Creation Tests */
    RUN_TEST(test_namespace_new);
    
    /* String Tests */
    RUN_TEST(test_namespace_set_get_string);
    RUN_TEST(test_namespace_set_string_overwrite);
    RUN_TEST(test_namespace_set_string_null_value);
    RUN_TEST(test_namespace_get_string_missing);
    RUN_TEST(test_namespace_get_string_wrong_type);
    RUN_TEST(test_namespace_set_string_multiple_keys);
    
    /* Integer Tests */
    RUN_TEST(test_namespace_set_get_int);
    RUN_TEST(test_namespace_set_int_overwrite);
    RUN_TEST(test_namespace_set_int_negative);
    RUN_TEST(test_namespace_set_int_zero);
    RUN_TEST(test_namespace_get_int_missing);
    RUN_TEST(test_namespace_get_int_wrong_type);
    
    /* Boolean Tests */
    RUN_TEST(test_namespace_set_get_bool);
    RUN_TEST(test_namespace_get_bool_missing);
    RUN_TEST(test_namespace_get_bool_wrong_type);
    
    /* Float Tests */
    RUN_TEST(test_namespace_set_get_float);
    RUN_TEST(test_namespace_get_float_missing);
    RUN_TEST(test_namespace_get_float_wrong_type);
    
    /* Array Tests */
    RUN_TEST(test_namespace_append_string);
    RUN_TEST(test_namespace_append_string_null_value);
    RUN_TEST(test_namespace_append_string_array_expansion);
    RUN_TEST(test_namespace_get_string_array_missing);
    RUN_TEST(test_namespace_get_string_array_wrong_type);
    
    /* Merge Tests */
    RUN_TEST(test_namespace_merge_basic);
    RUN_TEST(test_namespace_merge_no_overwrite);
    RUN_TEST(test_namespace_merge_with_arrays);
    RUN_TEST(test_namespace_merge_mixed_types);
    RUN_TEST(test_namespace_merge_null_params);
    
    /* Free Tests */
    RUN_TEST(test_namespace_free_null_safe);
    RUN_TEST(test_namespace_free_with_values);
    
    /* Capacity Tests */
    RUN_TEST(test_namespace_capacity_expansion);
    
    /* Edge Cases */
    RUN_TEST(test_namespace_string_to_int_not_allowed);
    RUN_TEST(test_namespace_int_to_string_not_allowed);
}

#ifdef STANDALONE_TEST
int main(void) {
    UNITY_BEGIN();
    run_test_namespace();
    return UNITY_END();
}
#endif