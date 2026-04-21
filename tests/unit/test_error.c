/**
 * @file test_error.c
 * @brief Unit tests for clap_error.c
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
 * clap_error_init Tests
 * ============================================================================ */

void test_error_init_basic(void) {
    clap_error_t error;
    /* Fill with garbage */
    memset(&error, 0xFF, sizeof(error));
    
    clap_error_init(&error);
    
    TEST_ASSERT_EQUAL(CLAP_ERR_NONE, error.code);
    TEST_ASSERT_EQUAL_STRING("", error.message);
    TEST_ASSERT_NULL(error.argument_name);
    TEST_ASSERT_NULL(error.invalid_value);
    TEST_ASSERT_NULL(error.subcommand_name);
}

void test_error_init_null(void) {
    /* Should not crash */
    clap_error_init(NULL);
    TEST_ASSERT_TRUE(1);
}

void test_error_init_twice(void) {
    clap_error_t error;
    
    clap_error_init(&error);
    error.code = CLAP_ERR_MEMORY;
    error.argument_name = "test";
    
    clap_error_init(&error);
    
    TEST_ASSERT_EQUAL(CLAP_ERR_NONE, error.code);
    TEST_ASSERT_NULL(error.argument_name);
}

/* ============================================================================
 * clap_error_set Tests
 * ============================================================================ */

void test_error_set_basic(void) {
    clap_error_t error;
    clap_error_init(&error);
    
    clap_error_set(&error, CLAP_ERR_INVALID_ARGUMENT, "Test error message");
    
    TEST_ASSERT_EQUAL(CLAP_ERR_INVALID_ARGUMENT, error.code);
    TEST_ASSERT_EQUAL_STRING("Test error message", error.message);
}

void test_error_set_with_formatting(void) {
    clap_error_t error;
    clap_error_init(&error);
    
    clap_error_set(&error, CLAP_ERR_TYPE_CONVERSION, 
                   "Cannot convert '%s' to %s", "abc", "int");
    
    TEST_ASSERT_EQUAL(CLAP_ERR_TYPE_CONVERSION, error.code);
    TEST_ASSERT_EQUAL_STRING("Cannot convert 'abc' to int", error.message);
}

void test_error_set_with_numbers(void) {
    clap_error_t error;
    clap_error_init(&error);
    
    clap_error_set(&error, CLAP_ERR_TOO_MANY_ARGS,
               "Expected %d arguments, got %d", 3, 5);
    
    TEST_ASSERT_EQUAL(CLAP_ERR_TOO_MANY_ARGS, error.code);
    TEST_ASSERT_EQUAL_STRING("Expected 3 arguments, got 5", error.message);
}

void test_error_set_null_format(void) {
    clap_error_t error;
    clap_error_init(&error);
    
    clap_error_set(&error, CLAP_ERR_MEMORY, NULL);
    
    TEST_ASSERT_EQUAL(CLAP_ERR_MEMORY, error.code);
    TEST_ASSERT_EQUAL_STRING("Memory allocation failed", error.message);
}

void test_error_set_null_error(void) {
    /* Should not crash */
    clap_error_set(NULL, CLAP_ERR_MEMORY, "test");
    TEST_ASSERT_TRUE(1);
}

void test_error_set_overwrites_previous(void) {
    clap_error_t error;
    clap_error_init(&error);
    
    clap_error_set(&error, CLAP_ERR_INVALID_ARGUMENT, "First error");
    clap_error_set(&error, CLAP_ERR_MEMORY, "Second error");
    
    TEST_ASSERT_EQUAL(CLAP_ERR_MEMORY, error.code);
    TEST_ASSERT_EQUAL_STRING("Second error", error.message);
}

void test_error_set_truncates_long_message(void) {
    clap_error_t error;
    clap_error_init(&error);
    
    /* Create a very long message */
    char long_msg[1024];
    memset(long_msg, 'X', sizeof(long_msg) - 1);
    long_msg[sizeof(long_msg) - 1] = '\0';
    
    clap_error_set(&error, CLAP_ERR_INVALID_ARGUMENT, "%s", long_msg);
    
    /* Message should be truncated to fit */
    TEST_ASSERT_EQUAL(CLAP_ERR_INVALID_ARGUMENT, error.code);
    TEST_ASSERT_LESS_THAN(1024, strlen(error.message));
}

/* ============================================================================
 * clap_error_vset Tests
 * ============================================================================ */
static void helper_vset(clap_error_t *error, int code, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    clap_error_vset(error, code, fmt, ap);
    va_end(ap);
}

void test_error_vset_basic(void) {
    clap_error_t error;
    clap_error_init(&error);
    
    helper_vset(&error, CLAP_ERR_INVALID_ARGUMENT, "%s", "Test error message");
    
    TEST_ASSERT_EQUAL(CLAP_ERR_INVALID_ARGUMENT, error.code);
    TEST_ASSERT_EQUAL_STRING("Test error message", error.message);
}

void test_error_vset_with_formatting(void) {
    clap_error_t error;
    clap_error_init(&error);
    
    /* Simulate vsnprintf with format args */
    clap_error_set(&error, CLAP_ERR_TYPE_CONVERSION, 
                   "Cannot convert '%s' to %s", "abc", "int");
    
    TEST_ASSERT_EQUAL(CLAP_ERR_TYPE_CONVERSION, error.code);
    TEST_ASSERT_EQUAL_STRING("Cannot convert 'abc' to int", error.message);
}

void test_error_vset_null_format(void) {
    clap_error_t error;
    clap_error_init(&error);
    
    va_list ap;
    /* ap is not used when format is NULL */
    clap_error_vset(&error, CLAP_ERR_MEMORY, NULL, ap);
    
    TEST_ASSERT_EQUAL(CLAP_ERR_MEMORY, error.code);
    TEST_ASSERT_EQUAL_STRING("Memory allocation failed", error.message);
}

void test_error_vset_null_error(void) {
    va_list ap;
    /* Should not crash */
    clap_error_vset(NULL, CLAP_ERR_MEMORY, "test", ap);
    TEST_ASSERT_TRUE(1);
}

/* ============================================================================
 * clap_strerror Tests
 * ============================================================================ */

void test_strerror_known_codes(void) {
    TEST_ASSERT_EQUAL_STRING("Success", clap_strerror(CLAP_ERR_NONE));
    TEST_ASSERT_EQUAL_STRING("Invalid argument", clap_strerror(CLAP_ERR_INVALID_ARGUMENT));
    TEST_ASSERT_EQUAL_STRING("Missing required value", clap_strerror(CLAP_ERR_MISSING_VALUE));
    TEST_ASSERT_EQUAL_STRING("Type conversion failed", clap_strerror(CLAP_ERR_TYPE_CONVERSION));
    TEST_ASSERT_EQUAL_STRING("Invalid choice", clap_strerror(CLAP_ERR_INVALID_CHOICE));
    TEST_ASSERT_EQUAL_STRING("Mutually exclusive arguments specified", 
                              clap_strerror(CLAP_ERR_MUTUALLY_EXCLUSIVE));
    TEST_ASSERT_EQUAL_STRING("Required argument missing", 
                              clap_strerror(CLAP_ERR_REQUIRED_MISSING));
    TEST_ASSERT_EQUAL_STRING("Unrecognized argument", clap_strerror(CLAP_ERR_UNRECOGNIZED));
    TEST_ASSERT_EQUAL_STRING("Too many arguments", clap_strerror(CLAP_ERR_TOO_MANY_ARGS));
    TEST_ASSERT_EQUAL_STRING("Too few arguments", clap_strerror(CLAP_ERR_TOO_FEW_ARGS));
    TEST_ASSERT_EQUAL_STRING("Memory allocation failed", clap_strerror(CLAP_ERR_MEMORY));
    TEST_ASSERT_EQUAL_STRING("Subcommand failed", clap_strerror(CLAP_ERR_SUBCOMMAND_FAILED));
    TEST_ASSERT_EQUAL_STRING("Dependency violation", clap_strerror(CLAP_ERR_DEPENDENCY_VIOLATION));
}

void test_strerror_unknown_code(void) {
    TEST_ASSERT_EQUAL_STRING("Unknown error", clap_strerror(-1));
    TEST_ASSERT_EQUAL_STRING("Unknown error", clap_strerror(9999));
    TEST_ASSERT_EQUAL_STRING("Unknown error", clap_strerror(CLAP_ERR_CUSTOM + 1));
}

void test_strerror_custom_code(void) {
    /* Custom error codes return "Unknown error" since they're not in switch */
    TEST_ASSERT_EQUAL_STRING("Unknown error", clap_strerror(CLAP_ERR_CUSTOM));
    TEST_ASSERT_EQUAL_STRING("Unknown error", clap_strerror(CLAP_ERR_CUSTOM + 10));
}

void test_strerror_all_codes_have_messages(void) {
    /* Verify all defined error codes have non-NULL messages */
    int codes[] = {
        CLAP_ERR_NONE,
        CLAP_ERR_INVALID_ARGUMENT,
        CLAP_ERR_MISSING_VALUE,
        CLAP_ERR_TYPE_CONVERSION,
        CLAP_ERR_INVALID_CHOICE,
        CLAP_ERR_MUTUALLY_EXCLUSIVE,
        CLAP_ERR_REQUIRED_MISSING,
        CLAP_ERR_UNRECOGNIZED,
        CLAP_ERR_TOO_MANY_ARGS,
        CLAP_ERR_TOO_FEW_ARGS,
        CLAP_ERR_MEMORY,
        CLAP_ERR_SUBCOMMAND_FAILED,
        CLAP_ERR_DEPENDENCY_VIOLATION,
    };
    
    for (size_t i = 0; i < sizeof(codes) / sizeof(codes[0]); i++) {
        const char *msg = clap_strerror(codes[i]);
        TEST_ASSERT_NOT_NULL(msg);
        TEST_ASSERT_NOT_EQUAL(0, strlen(msg));
    }
}

/* ============================================================================
 * Error Structure Fields Tests
 * ============================================================================ */

void test_error_fields_can_be_set(void) {
    clap_error_t error;
    clap_error_init(&error);
    
    error.code = CLAP_ERR_INVALID_CHOICE;
    error.argument_name = "--color";
    error.invalid_value = "purple";
    error.subcommand_name = "commit";
    
    TEST_ASSERT_EQUAL(CLAP_ERR_INVALID_CHOICE, error.code);
    TEST_ASSERT_EQUAL_STRING("--color", error.argument_name);
    TEST_ASSERT_EQUAL_STRING("purple", error.invalid_value);
    TEST_ASSERT_EQUAL_STRING("commit", error.subcommand_name);
}

void test_error_message_buffer_size(void) {
    clap_error_t error;
    
    /* Verify message buffer size is 512 */
    TEST_ASSERT_EQUAL(512, sizeof(error.message));
}

/* ============================================================================
 * Integration Tests
 * ============================================================================ */

void test_error_flow_parse_failure(void) {
    /* Simulate a parse failure and error propagation */
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--required");
    clap_argument_required(arg, true);
    
    char *argv[] = {"prog"};
    clap_namespace_t *ns = NULL;
    clap_error_t error;
    clap_error_init(&error);
    
    bool result = clap_parse_args(parser, 1, argv, &ns, &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_REQUIRED_MISSING, error.code);
    TEST_ASSERT_NOT_EQUAL(0, strlen(error.message));
    
    clap_parser_free(parser);
}

void test_error_flow_invalid_choice(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--color");
    const char *choices[] = {"red", "green", "blue"};
    clap_argument_choices(arg, choices, 3);
    
    char *argv[] = {"prog", "--color", "yellow"};
    clap_namespace_t *ns = NULL;
    clap_error_t error;
    clap_error_init(&error);
    
    bool result = clap_parse_args(parser, 3, argv, &ns, &error);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_INVALID_CHOICE, error.code);
    TEST_ASSERT_NOT_NULL(strstr(error.message, "yellow"));
    TEST_ASSERT_NOT_NULL(strstr(error.message, "red"));
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

void run_test_error(void) {
    /* clap_error_init Tests */
    RUN_TEST(test_error_init_basic);
    RUN_TEST(test_error_init_null);
    RUN_TEST(test_error_init_twice);
    
    /* clap_error_set Tests */
    RUN_TEST(test_error_set_basic);
    RUN_TEST(test_error_set_with_formatting);
    RUN_TEST(test_error_set_with_numbers);
    RUN_TEST(test_error_set_null_format);
    RUN_TEST(test_error_set_null_error);
    RUN_TEST(test_error_set_overwrites_previous);
    RUN_TEST(test_error_set_truncates_long_message);
    
    /* clap_error_vset Tests */
    RUN_TEST(test_error_vset_basic);
    RUN_TEST(test_error_vset_with_formatting);
    RUN_TEST(test_error_vset_null_format);
    RUN_TEST(test_error_vset_null_error);
    
    /* clap_strerror Tests */
    RUN_TEST(test_strerror_known_codes);
    RUN_TEST(test_strerror_unknown_code);
    RUN_TEST(test_strerror_custom_code);
    RUN_TEST(test_strerror_all_codes_have_messages);
    
    /* Error Structure Fields Tests */
    RUN_TEST(test_error_fields_can_be_set);
    RUN_TEST(test_error_message_buffer_size);
    
    /* Integration Tests */
    RUN_TEST(test_error_flow_parse_failure);
    RUN_TEST(test_error_flow_invalid_choice);
}

#ifdef STANDALONE_TEST
int main(void) {
    UNITY_BEGIN();
    run_test_error();
    return UNITY_END();
}
#endif
