/**
 * @file test_action_executor.c
 * @brief Unit tests for clap_action_executor.c
 */

#include "unity.h"
#include "clap_parser_internal.h"

void setUp(void) {
    /* Nothing needed */
}

void tearDown(void) {
    /* Nothing needed */
}

void test_apply_argument_action_store_with_value(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--test");
    clap_argument_dest(arg, "test");
    clap_namespace_t *ns = clap_namespace_new();
    clap_error_t error = {0};

    bool result = clap_apply_argument_action(parser, arg, ns, "hello", &error);

    TEST_ASSERT_TRUE(result);
    const char *value;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "test", &value));
    TEST_ASSERT_EQUAL_STRING("hello", value);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_apply_argument_action_store_without_value(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--test");
    clap_argument_dest(arg, "test");
    clap_namespace_t *ns = clap_namespace_new();
    clap_error_t error = {0};

    bool result = clap_apply_argument_action(parser, arg, ns, NULL, &error);

    TEST_ASSERT_TRUE(result);
    const char *value;
    TEST_ASSERT_FALSE(clap_namespace_get_string(ns, "test", &value));

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_apply_argument_action_store_true(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--verbose");
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    clap_argument_dest(arg, "verbose");
    clap_namespace_t *ns = clap_namespace_new();
    clap_error_t error = {0};

    bool result = clap_apply_argument_action(parser, arg, ns, NULL, &error);

    TEST_ASSERT_TRUE(result);
    bool value;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "verbose", &value));
    TEST_ASSERT_TRUE(value);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_apply_argument_action_store_false(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--quiet");
    clap_argument_action(arg, CLAP_ACTION_STORE_FALSE);
    clap_argument_dest(arg, "verbose");
    clap_namespace_t *ns = clap_namespace_new();
    clap_error_t error = {0};

    bool result = clap_apply_argument_action(parser, arg, ns, NULL, &error);

    TEST_ASSERT_TRUE(result);
    bool value;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "verbose", &value));
    TEST_ASSERT_FALSE(value);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_apply_argument_action_store_const(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--mode");
    clap_argument_action(arg, CLAP_ACTION_STORE_CONST);
    clap_argument_const(arg, "fast");
    clap_argument_dest(arg, "mode");
    clap_namespace_t *ns = clap_namespace_new();
    clap_error_t error = {0};

    bool result = clap_apply_argument_action(parser, arg, ns, NULL, &error);

    TEST_ASSERT_TRUE(result);
    const char *value;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "mode", &value));
    TEST_ASSERT_EQUAL_STRING("fast", value);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_apply_argument_action_store_const_missing_const(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--mode");
    clap_argument_action(arg, CLAP_ACTION_STORE_CONST);
    clap_argument_dest(arg, "mode");
    clap_namespace_t *ns = clap_namespace_new();
    clap_error_t error = {0};

    bool result = clap_apply_argument_action(parser, arg, ns, NULL, &error);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_INVALID_ARGUMENT, error.code);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_apply_argument_action_append(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--tag");
    clap_argument_action(arg, CLAP_ACTION_APPEND);
    clap_argument_dest(arg, "tags");
    clap_namespace_t *ns = clap_namespace_new();
    clap_error_t error = {0};

    clap_apply_argument_action(parser, arg, ns, "important", &error);
    clap_apply_argument_action(parser, arg, ns, "urgent", &error);

    const char **tags;
    size_t count;
    TEST_ASSERT_TRUE(clap_namespace_get_string_array(ns, "tags", &tags, &count));
    TEST_ASSERT_EQUAL(2, count);
    TEST_ASSERT_EQUAL_STRING("important", tags[0]);
    TEST_ASSERT_EQUAL_STRING("urgent", tags[1]);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_apply_argument_action_append_const(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--add-item");
    clap_argument_action(arg, CLAP_ACTION_APPEND_CONST);
    clap_argument_const(arg, "item");
    clap_argument_dest(arg, "items");
    clap_namespace_t *ns = clap_namespace_new();
    clap_error_t error = {0};

    clap_apply_argument_action(parser, arg, ns, NULL, &error);
    clap_apply_argument_action(parser, arg, ns, NULL, &error);

    const char **items;
    size_t count;
    TEST_ASSERT_TRUE(clap_namespace_get_string_array(ns, "items", &items, &count));
    TEST_ASSERT_EQUAL(2, count);
    TEST_ASSERT_EQUAL_STRING("item", items[0]);
    TEST_ASSERT_EQUAL_STRING("item", items[1]);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_apply_argument_action_count(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "-v");
    clap_argument_action(arg, CLAP_ACTION_COUNT);
    clap_argument_dest(arg, "verbose");
    clap_namespace_t *ns = clap_namespace_new();
    clap_error_t error = {0};

    clap_apply_argument_action(parser, arg, ns, NULL, &error);
    clap_apply_argument_action(parser, arg, ns, NULL, &error);
    clap_apply_argument_action(parser, arg, ns, NULL, &error);

    int count;
    TEST_ASSERT_TRUE(clap_namespace_get_int(ns, "verbose", &count));
    TEST_ASSERT_EQUAL(3, count);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

static bool test_custom_action_handler(clap_parser_t *parser,
                                       clap_argument_t *arg,
                                       clap_namespace_t *ns,
                                       const char **values,
                                       size_t count,
                                       void *user_data,
                                       clap_error_t *error) {
    (void)parser;
    (void)arg;
    (void)user_data;
    (void)error;
    if (count == 1) {
        return clap_namespace_set_string(ns, "custom", values[0]);
    }
    return false;
}

void test_apply_argument_action_custom(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--custom");
    clap_argument_action(arg, CLAP_ACTION_CUSTOM);
    clap_argument_dest(arg, "custom");
    clap_argument_handler(arg, test_custom_action_handler);
    clap_namespace_t *ns = clap_namespace_new();
    clap_error_t error = {0};

    bool result = clap_apply_argument_action(parser, arg, ns, "value", &error);

    TEST_ASSERT_TRUE(result);
    const char *value;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "custom", &value));
    TEST_ASSERT_EQUAL_STRING("value", value);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void test_apply_argument_action_validates_choices(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--mode");
    clap_argument_dest(arg, "mode");

    const char *choices[] = {"fast", "safe"};
    clap_argument_choices(arg, choices, 2);
    clap_namespace_t *ns = clap_namespace_new();
    clap_error_t error = {0};

    bool result = clap_apply_argument_action(parser, arg, ns, "invalid", &error);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(CLAP_ERR_INVALID_CHOICE, error.code);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

void run_test_action_executor(void) {
    RUN_TEST(test_apply_argument_action_store_with_value);
    RUN_TEST(test_apply_argument_action_store_without_value);
    RUN_TEST(test_apply_argument_action_store_true);
    RUN_TEST(test_apply_argument_action_store_false);
    RUN_TEST(test_apply_argument_action_store_const);
    RUN_TEST(test_apply_argument_action_store_const_missing_const);
    RUN_TEST(test_apply_argument_action_append);
    RUN_TEST(test_apply_argument_action_append_const);
    RUN_TEST(test_apply_argument_action_count);
    RUN_TEST(test_apply_argument_action_custom);
    RUN_TEST(test_apply_argument_action_validates_choices);
}

#ifdef STANDALONE_TEST
int main(void) {
    UNITY_BEGIN();
    run_test_action_executor();
    return UNITY_END();
}
#endif
