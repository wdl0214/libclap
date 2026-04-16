/**
 * @file test_convert.c
 * @brief Unit tests for clap_convert.c
 */

#include "unity.h"
#include "clap_parser_internal.h"
#include <string.h>
#include <errno.h>
#include <float.h>

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
 * clap_type_string_handler Tests
 * ============================================================================ */

void test_type_string_basic(void) {
    const char *input = "hello world";
    const char *output = NULL;
    clap_error_t error = {0};
    
    bool result = clap_type_string_handler(input, &output, sizeof(char**), &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_PTR(input, output);
    TEST_ASSERT_EQUAL_STRING("hello world", output);
}

void test_type_string_empty(void) {
    const char *input = "";
    const char *output = NULL;
    clap_error_t error = {0};
    
    bool result = clap_type_string_handler(input, &output, sizeof(char**), &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_STRING("", output);
}

void test_type_string_null_input(void) {
    const char *output = NULL;
    clap_error_t error = {0};
    
    bool result = clap_type_string_handler(NULL, &output, sizeof(char**), &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_NULL(output);
}

void test_type_string_invalid_output_size(void) {
    const char *input = "test";
    const char *output = NULL;
    clap_error_t error = {0};
    
    bool result = clap_type_string_handler(input, &output, sizeof(int), &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_TYPE_CONVERSION, error.code);
    TEST_ASSERT_NOT_NULL(strstr(error.message, "Invalid output size"));
}

/* ============================================================================
 * clap_type_int_handler Tests
 * ============================================================================ */

void test_type_int_positive(void) {
    int output = 0;
    clap_error_t error = {0};
    
    bool result = clap_type_int_handler("42", &output, sizeof(int), &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(42, output);
}

void test_type_int_negative(void) {
    int output = 0;
    clap_error_t error = {0};
    
    bool result = clap_type_int_handler("-100", &output, sizeof(int), &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(-100, output);
}

void test_type_int_zero(void) {
    int output = 1;
    clap_error_t error = {0};
    
    bool result = clap_type_int_handler("0", &output, sizeof(int), &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, output);
}

void test_type_int_hex(void) {
    int output = 0;
    clap_error_t error = {0};
    
    bool result = clap_type_int_handler("0xFF", &output, sizeof(int), &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(255, output);
}

void test_type_int_octal(void) {
    int output = 0;
    clap_error_t error = {0};
    
    bool result = clap_type_int_handler("077", &output, sizeof(int), &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(63, output);
}

void test_type_int_max(void) {
    int output = 0;
    clap_error_t error = {0};
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%d", INT_MAX);
    
    bool result = clap_type_int_handler(buffer, &output, sizeof(int), &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(INT_MAX, output);
}

void test_type_int_min(void) {
    int output = 0;
    clap_error_t error = {0};
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%d", INT_MIN);
    
    bool result = clap_type_int_handler(buffer, &output, sizeof(int), &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(INT_MIN, output);
}

void test_type_int_overflow(void) {
    int output = 0;
    clap_error_t error = {0};
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%ld", (long)INT_MAX + 1);
    
    bool result = clap_type_int_handler(buffer, &output, sizeof(int), &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_TYPE_CONVERSION, error.code);
    TEST_ASSERT_NOT_NULL(strstr(error.message, "out of range"));
}

void test_type_int_underflow(void) {
    int output = 0;
    clap_error_t error = {0};
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%ld", (long)INT_MIN - 1);
    
    bool result = clap_type_int_handler(buffer, &output, sizeof(int), &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_TYPE_CONVERSION, error.code);
}

void test_type_int_invalid_format(void) {
    int output = 0;
    clap_error_t error = {0};
    
    bool result = clap_type_int_handler("abc", &output, sizeof(int), &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_TYPE_CONVERSION, error.code);
    TEST_ASSERT_NOT_NULL(strstr(error.message, "Invalid integer"));
}

void test_type_int_partial_valid(void) {
    int output = 0;
    clap_error_t error = {0};
    
    bool result = clap_type_int_handler("42abc", &output, sizeof(int), &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_TYPE_CONVERSION, error.code);
}

void test_type_int_with_spaces(void) {
    int output = 0;
    clap_error_t error = {0};
    
    /* strtol handles leading spaces, but not trailing */
    bool result = clap_type_int_handler("  42", &output, sizeof(int), &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(42, output);
}

void test_type_int_invalid_output_size(void) {
    int output = 0;
    clap_error_t error = {0};
    
    bool result = clap_type_int_handler("42", &output, sizeof(long), &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_TYPE_CONVERSION, error.code);
}

/* ============================================================================
 * clap_type_float_handler Tests
 * ============================================================================ */

void test_type_float_positive(void) {
    double output = 0.0;
    clap_error_t error = {0};
    
    bool result = clap_type_float_handler("3.14159", &output, sizeof(double), &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_DOUBLE(3.14159, output);
}

void test_type_float_negative(void) {
    double output = 0.0;
    clap_error_t error = {0};
    
    bool result = clap_type_float_handler("-2.5", &output, sizeof(double), &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_DOUBLE(-2.5, output);
}

void test_type_float_zero(void) {
    double output = 1.0;
    clap_error_t error = {0};
    
    bool result = clap_type_float_handler("0.0", &output, sizeof(double), &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, output);
}

void test_type_float_integer(void) {
    double output = 0.0;
    clap_error_t error = {0};
    
    bool result = clap_type_float_handler("42", &output, sizeof(double), &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_DOUBLE(42.0, output);
}

void test_type_float_scientific(void) {
    double output = 0.0;
    clap_error_t error = {0};
    
    bool result = clap_type_float_handler("1.23e-4", &output, sizeof(double), &error);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_DOUBLE(0.000123, output);
}

void test_type_float_overflow(void) {
    double output = 0.0;
    clap_error_t error = {0};
    
    bool result = clap_type_float_handler("1e999", &output, sizeof(double), &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_TYPE_CONVERSION, error.code);
    TEST_ASSERT_NOT_NULL(strstr(error.message, "out of range"));
}

void test_type_float_invalid_format(void) {
    double output = 0.0;
    clap_error_t error = {0};
    
    bool result = clap_type_float_handler("abc", &output, sizeof(double), &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_TYPE_CONVERSION, error.code);
    TEST_ASSERT_NOT_NULL(strstr(error.message, "Invalid float"));
}

void test_type_float_nan(void) {
    double output = 0.0;
    clap_error_t error = {0};
    
    bool result = clap_type_float_handler("NaN", &output, sizeof(double), &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_TYPE_CONVERSION, error.code);
    TEST_ASSERT_NOT_NULL(strstr(error.message, "NaN"));
}

void test_type_float_infinity(void) {
    double output = 0.0;
    clap_error_t error = {0};
    
    bool result = clap_type_float_handler("inf", &output, sizeof(double), &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_TYPE_CONVERSION, error.code);
    TEST_ASSERT_NOT_NULL(strstr(error.message, "Infinity"));
}

void test_type_float_invalid_output_size(void) {
    double output = 0.0;
    clap_error_t error = {0};
    
    bool result = clap_type_float_handler("3.14", &output, sizeof(float), &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_TYPE_CONVERSION, error.code);
}

/* ============================================================================
 * clap_type_bool_handler Tests
 * ============================================================================ */

void test_type_bool_true_variants(void) {
    bool output = false;
    clap_error_t error = {0};
    
    TEST_ASSERT_TRUE(clap_type_bool_handler("true", &output, sizeof(bool), &error));
    TEST_ASSERT_TRUE(output);
    
    TEST_ASSERT_TRUE(clap_type_bool_handler("yes", &output, sizeof(bool), &error));
    TEST_ASSERT_TRUE(output);
    
    TEST_ASSERT_TRUE(clap_type_bool_handler("1", &output, sizeof(bool), &error));
    TEST_ASSERT_TRUE(output);
    
    TEST_ASSERT_TRUE(clap_type_bool_handler("on", &output, sizeof(bool), &error));
    TEST_ASSERT_TRUE(output);
}

void test_type_bool_true_uppercase(void) {
    bool output = false;
    clap_error_t error = {0};
    
    /* Case-sensitive - should fail */
    TEST_ASSERT_FALSE(clap_type_bool_handler("TRUE", &output, sizeof(bool), &error));
    TEST_ASSERT_FALSE(clap_type_bool_handler("YES", &output, sizeof(bool), &error));
}

void test_type_bool_false_variants(void) {
    bool output = true;
    clap_error_t error = {0};
    
    TEST_ASSERT_TRUE(clap_type_bool_handler("false", &output, sizeof(bool), &error));
    TEST_ASSERT_FALSE(output);
    
    TEST_ASSERT_TRUE(clap_type_bool_handler("no", &output, sizeof(bool), &error));
    TEST_ASSERT_FALSE(output);
    
    TEST_ASSERT_TRUE(clap_type_bool_handler("0", &output, sizeof(bool), &error));
    TEST_ASSERT_FALSE(output);
    
    TEST_ASSERT_TRUE(clap_type_bool_handler("off", &output, sizeof(bool), &error));
    TEST_ASSERT_FALSE(output);
}

void test_type_bool_invalid(void) {
    bool output = false;
    clap_error_t error = {0};
    
    bool result = clap_type_bool_handler("maybe", &output, sizeof(bool), &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_TYPE_CONVERSION, error.code);
    TEST_ASSERT_NOT_NULL(strstr(error.message, "Invalid boolean"));
}

void test_type_bool_empty(void) {
    bool output = false;
    clap_error_t error = {0};
    
    bool result = clap_type_bool_handler("", &output, sizeof(bool), &error);
    
    TEST_ASSERT_FALSE(result);
}

void test_type_bool_invalid_output_size(void) {
    bool output = false;
    clap_error_t error = {0};
    
    bool result = clap_type_bool_handler("true", &output, sizeof(int), &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_TYPE_CONVERSION, error.code);
}

/* ============================================================================
 * clap_apply_defaults Tests
 * ============================================================================ */

void test_apply_defaults_string(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "--output");
    clap_argument_default(arg, "stdout");
    clap_argument_dest(arg, "output");
    
    bool result = clap_apply_defaults(g_parser, g_ns, NULL);
    
    TEST_ASSERT_TRUE(result);
    
    const char *value;
    TEST_ASSERT_TRUE(clap_namespace_get_string(g_ns, "output", &value));
    TEST_ASSERT_EQUAL_STRING("stdout", value);
}

void test_apply_defaults_store_true(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "--verbose");
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    clap_argument_dest(arg, "verbose");
    
    clap_apply_defaults(g_parser, g_ns, NULL);
    
    bool value;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(g_ns, "verbose", &value));
    TEST_ASSERT_FALSE(value);
}

void test_apply_defaults_store_false(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "--quiet");
    clap_argument_action(arg, CLAP_ACTION_STORE_FALSE);
    clap_argument_dest(arg, "quiet");
    
    clap_apply_defaults(g_parser, g_ns, NULL);
    
    bool value;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(g_ns, "quiet", &value));
    TEST_ASSERT_TRUE(value);
}

void test_apply_defaults_count(void) {
    clap_argument_t *arg = clap_add_argument(g_parser, "-v");
    clap_argument_action(arg, CLAP_ACTION_COUNT);
    clap_argument_dest(arg, "verbose");
    
    clap_apply_defaults(g_parser, g_ns, NULL);
    
    int value;
    TEST_ASSERT_TRUE(clap_namespace_get_int(g_ns, "verbose", &value));
    TEST_ASSERT_EQUAL(0, value);
}

void test_apply_defaults_multiple(void) {
    clap_argument_t *out = clap_add_argument(g_parser, "--output");
    clap_argument_default(out, "stdout");
    clap_argument_dest(out, "output");
    
    clap_argument_t *verbose = clap_add_argument(g_parser, "--verbose");
    clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);
    clap_argument_dest(verbose, "verbose");
    
    clap_argument_t *count = clap_add_argument(g_parser, "-c");
    clap_argument_action(count, CLAP_ACTION_COUNT);
    clap_argument_dest(count, "count");
    
    clap_apply_defaults(g_parser, g_ns, NULL);
    
    const char *out_val;
    bool verbose_val;
    int count_val;
    
    TEST_ASSERT_TRUE(clap_namespace_get_string(g_ns, "output", &out_val));
    TEST_ASSERT_EQUAL_STRING("stdout", out_val);
    
    TEST_ASSERT_TRUE(clap_namespace_get_bool(g_ns, "verbose", &verbose_val));
    TEST_ASSERT_FALSE(verbose_val);
    
    TEST_ASSERT_TRUE(clap_namespace_get_int(g_ns, "count", &count_val));
    TEST_ASSERT_EQUAL(0, count_val);
}

void test_apply_defaults_no_defaults(void) {
    clap_add_argument(g_parser, "--output");
    clap_add_argument(g_parser, "input");
    
    bool result = clap_apply_defaults(g_parser, g_ns, NULL);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, g_ns->value_count);
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

void run_test_convert(void) {
    /* String Handler Tests */
    RUN_TEST(test_type_string_basic);
    RUN_TEST(test_type_string_empty);
    RUN_TEST(test_type_string_null_input);
    RUN_TEST(test_type_string_invalid_output_size);
    
    /* Int Handler Tests */
    RUN_TEST(test_type_int_positive);
    RUN_TEST(test_type_int_negative);
    RUN_TEST(test_type_int_zero);
    RUN_TEST(test_type_int_hex);
    RUN_TEST(test_type_int_octal);
    RUN_TEST(test_type_int_max);
    RUN_TEST(test_type_int_min);
    RUN_TEST(test_type_int_overflow);
    RUN_TEST(test_type_int_underflow);
    RUN_TEST(test_type_int_invalid_format);
    RUN_TEST(test_type_int_partial_valid);
    RUN_TEST(test_type_int_with_spaces);
    RUN_TEST(test_type_int_invalid_output_size);
    
    /* Float Handler Tests */
    RUN_TEST(test_type_float_positive);
    RUN_TEST(test_type_float_negative);
    RUN_TEST(test_type_float_zero);
    RUN_TEST(test_type_float_integer);
    RUN_TEST(test_type_float_scientific);
    RUN_TEST(test_type_float_overflow);
    RUN_TEST(test_type_float_invalid_format);
    RUN_TEST(test_type_float_nan);
    RUN_TEST(test_type_float_infinity);
    RUN_TEST(test_type_float_invalid_output_size);
    
    /* Bool Handler Tests */
    RUN_TEST(test_type_bool_true_variants);
    RUN_TEST(test_type_bool_true_uppercase);
    RUN_TEST(test_type_bool_false_variants);
    RUN_TEST(test_type_bool_invalid);
    RUN_TEST(test_type_bool_empty);
    RUN_TEST(test_type_bool_invalid_output_size);
    
    /* Apply Defaults Tests */
    RUN_TEST(test_apply_defaults_string);
    RUN_TEST(test_apply_defaults_store_true);
    RUN_TEST(test_apply_defaults_store_false);
    RUN_TEST(test_apply_defaults_count);
    RUN_TEST(test_apply_defaults_multiple);
    RUN_TEST(test_apply_defaults_no_defaults);
}

#ifdef STANDALONE_TEST
int main(void) {
    UNITY_BEGIN();
    run_test_convert();
    return UNITY_END();
}
#endif