/**
 * @file test_clap.c
 * @brief Unit tests for clap.c
 */

#include "unity.h"
#include <clap/clap.h>
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
 * clap_version Tests
 * ============================================================================ */

void test_clap_version_format(void) {
    const char *version = clap_version();
    
    TEST_ASSERT_NOT_NULL(version);
    
    /* Should be in format "major.minor.patch" */
    int major, minor, patch;
    int parsed = sscanf(version, "%d.%d.%d", &major, &minor, &patch);
    
    TEST_ASSERT_EQUAL(3, parsed);
    TEST_ASSERT_EQUAL(1, major);
    TEST_ASSERT_EQUAL(0, minor);
    TEST_ASSERT_EQUAL(0, patch);
}

void test_clap_version_string_content(void) {
    const char *version = clap_version();
    
    TEST_ASSERT_EQUAL_STRING("1.0.0", version);
}

void test_clap_version_multiple_calls(void) {
    /* Multiple calls should return the same pointer */
    const char *v1 = clap_version();
    const char *v2 = clap_version();
    const char *v3 = clap_version();
    
    TEST_ASSERT_EQUAL_PTR(v1, v2);
    TEST_ASSERT_EQUAL_PTR(v2, v3);
    TEST_ASSERT_EQUAL_STRING("1.0.0", v1);
}

void test_clap_version_static_buffer_persistence(void) {
    const char *v1 = clap_version();
    
    /* The static buffer should persist */
    TEST_ASSERT_EQUAL_STRING("1.0.0", v1);
    
    /* Call again and verify content unchanged */
    const char *v2 = clap_version();
    TEST_ASSERT_EQUAL_STRING("1.0.0", v2);
}

void test_clap_version_buffer_size(void) {
    const char *version = clap_version();
    
    /* Version string should fit in reasonable buffer */
    TEST_ASSERT_LESS_THAN(32, strlen(version) + 1);
}

/* ============================================================================
 * Version Consistency Tests
 * ============================================================================ */

void test_clap_version_major_is_positive(void) {
    const char *version = clap_version();
    int major;
    sscanf(version, "%d", &major);
    
    TEST_ASSERT_GREATER_OR_EQUAL(1, major);
}

void test_clap_version_minor_is_non_negative(void) {
    const char *version = clap_version();
    int major, minor;
    sscanf(version, "%d.%d", &major, &minor);
    
    TEST_ASSERT_GREATER_OR_EQUAL(0, minor);
}

void test_clap_version_patch_is_non_negative(void) {
    const char *version = clap_version();
    int major, minor, patch;
    sscanf(version, "%d.%d.%d", &major, &minor, &patch);
    
    TEST_ASSERT_GREATER_OR_EQUAL(0, patch);
}

/* ============================================================================
 * Library Initialization Tests
 * ============================================================================ */

void test_clap_parser_new_after_version(void) {
    /* Version should work before parser creation */
    const char *v1 = clap_version();
    TEST_ASSERT_NOT_NULL(v1);
    
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    TEST_ASSERT_NOT_NULL(parser);
    
    /* Version should still work after parser creation */
    const char *v2 = clap_version();
    TEST_ASSERT_EQUAL_STRING(v1, v2);
    
    clap_parser_free(parser);
}

void test_clap_multiple_parsers_version_unchanged(void) {
    const char *v1 = clap_version();
    
    clap_parser_t *p1 = clap_parser_new("prog1", NULL, NULL);
    clap_parser_t *p2 = clap_parser_new("prog2", NULL, NULL);
    
    const char *v2 = clap_version();
    
    TEST_ASSERT_EQUAL_STRING(v1, v2);
    
    clap_parser_free(p2);
    clap_parser_free(p1);
}

/* ============================================================================
 * Edge Cases
 * ============================================================================ */

void test_clap_version_no_null_terminator_issue(void) {
    /* Verify the version string is properly null-terminated */
    const char *version = clap_version();
    size_t len = strlen(version);
    
    /* Should be exactly the length of "1.0.0" */
    TEST_ASSERT_EQUAL(5, len);
    TEST_ASSERT_EQUAL('\0', version[len]);
}

void test_clap_version_printable_only(void) {
    const char *version = clap_version();
    
    for (size_t i = 0; version[i]; i++) {
        /* All characters should be printable (digits or dots) */
        TEST_ASSERT_TRUE((version[i] >= '0' && version[i] <= '9') || version[i] == '.');
    }
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

void run_test_clap(void) {
    /* clap_version Tests */
    RUN_TEST(test_clap_version_format);
    RUN_TEST(test_clap_version_string_content);
    RUN_TEST(test_clap_version_multiple_calls);
    RUN_TEST(test_clap_version_static_buffer_persistence);
    RUN_TEST(test_clap_version_buffer_size);
    
    /* Version Consistency Tests */
    RUN_TEST(test_clap_version_major_is_positive);
    RUN_TEST(test_clap_version_minor_is_non_negative);
    RUN_TEST(test_clap_version_patch_is_non_negative);
    
    /* Library Initialization Tests */
    RUN_TEST(test_clap_parser_new_after_version);
    RUN_TEST(test_clap_multiple_parsers_version_unchanged);
    
    /* Edge Cases */
    RUN_TEST(test_clap_version_no_null_terminator_issue);
    RUN_TEST(test_clap_version_printable_only);
}

#ifdef STANDALONE_TEST
int main(void) {
    UNITY_BEGIN();
    run_test_clap();
    return UNITY_END();
}
#endif