/**
 * @file test_formatter.c
 * @brief Unit tests for clap_formatter.c
 */

/* Include source directly to access static functions */
#include "unity.h"
#include "../src/clap_formatter.c"

/* ============================================================================
 * Test Setup and Teardown
 * ============================================================================ */

/* Capture output for verification */
static char g_captured[65536];
static size_t g_captured_len = 0;

static void capture_write(void *cookie, const char *data, int size) {
    (void)cookie;
    if (g_captured_len + size < sizeof(g_captured) - 1) {
        memcpy(g_captured + g_captured_len, data, size);
        g_captured_len += size;
        g_captured[g_captured_len] = '\0';
    }
}

static FILE* capture_stdout(void) {
    g_captured_len = 0;
    g_captured[0] = '\0';
#ifdef _WIN32
    return stdout;
#else
    cookie_io_functions_t funcs = {
        .write = capture_write,
        .read = NULL,
        .seek = NULL,
        .close = NULL
    };
    return fopencookie(NULL, "w", funcs);
#endif
}

static const char* get_captured(void) {
    return g_captured;
}

void setUp(void) {
    g_captured_len = 0;
    g_captured[0] = '\0';
}

void tearDown(void) {
    /* Nothing needed */
}

/* ============================================================================
 * wrap_text Tests
 * ============================================================================ */

void test_wrap_text_short_text(void) {
    clap_buffer_t *buf = clap_buffer_empty();
    
    wrap_text(&buf, "Hello world", 0, 80, 0);
    
    TEST_ASSERT_EQUAL_STRING("Hello world", clap_buffer_cstr(buf));
    
    clap_buffer_free(buf);
}

void test_wrap_text_with_indent(void) {
    clap_buffer_t *buf = clap_buffer_empty();
    
    wrap_text(&buf, "Hello world", 4, 80, 0);
    
    TEST_ASSERT_EQUAL_STRING("    Hello world", clap_buffer_cstr(buf));
    
    clap_buffer_free(buf);
}

void test_wrap_text_long_text_wraps(void) {
    clap_buffer_t *buf = clap_buffer_empty();
    
    wrap_text(&buf, "This is a very long text that should wrap to multiple lines", 0, 20, 0);
    
    /* Should contain newline */
    TEST_ASSERT_NOT_NULL(strchr(clap_buffer_cstr(buf), '\n'));
    
    clap_buffer_free(buf);
}

void test_wrap_text_first_line_already_indented(void) {
    clap_buffer_t *buf = clap_buffer_empty();
    
    /* Simulate already having content on the line */
    clap_buffer_cat(&buf, "    ");
    
    wrap_text(&buf, "Hello world", 4, 80, 1);
    
    TEST_ASSERT_EQUAL_STRING("    Hello world", clap_buffer_cstr(buf));
    
    clap_buffer_free(buf);
}

void test_wrap_text_null_text(void) {
    clap_buffer_t *buf = clap_buffer_empty();
    
    wrap_text(&buf, NULL, 0, 80, 0);
    
    TEST_ASSERT_EQUAL_STRING("", clap_buffer_cstr(buf));
    
    clap_buffer_free(buf);
}

void test_wrap_text_empty_text(void) {
    clap_buffer_t *buf = clap_buffer_empty();
    
    wrap_text(&buf, "", 0, 80, 0);
    
    TEST_ASSERT_EQUAL_STRING("", clap_buffer_cstr(buf));
    
    clap_buffer_free(buf);
}

/* ============================================================================
 * get_upper_metavar Tests
 * ============================================================================ */

void test_get_upper_metavar_with_choices(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--color");
    const char *choices[] = {"red", "green", "blue"};
    clap_argument_choices(arg, choices, 3);
    
    char out[64];
    get_upper_metavar(arg, out, sizeof(out));
    
    TEST_ASSERT_EQUAL_STRING("{red,green,blue}", out);
    
    clap_parser_free(parser);
}

void test_get_upper_metavar_with_metavar(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--output");
    clap_argument_metavar(arg, "FILE");
    
    char out[64];
    get_upper_metavar(arg, out, sizeof(out));
    
    TEST_ASSERT_EQUAL_STRING("FILE", out);
    
    clap_parser_free(parser);
}

void test_get_upper_metavar_with_dest(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--output");
    clap_argument_dest(arg, "output_file");
    
    char out[64];
    get_upper_metavar(arg, out, sizeof(out));
    
    TEST_ASSERT_EQUAL_STRING("OUTPUT_FILE", out);
    
    clap_parser_free(parser);
}

/* ============================================================================
 * action_requires_value Tests
 * ============================================================================ */

void test_action_requires_value_store(void) {
    TEST_ASSERT_TRUE(action_requires_value(CLAP_ACTION_STORE));
}

void test_action_requires_value_append(void) {
    TEST_ASSERT_TRUE(action_requires_value(CLAP_ACTION_APPEND));
}

void test_action_requires_value_store_true(void) {
    TEST_ASSERT_FALSE(action_requires_value(CLAP_ACTION_STORE_TRUE));
}

void test_action_requires_value_store_false(void) {
    TEST_ASSERT_FALSE(action_requires_value(CLAP_ACTION_STORE_FALSE));
}

void test_action_requires_value_store_const(void) {
    TEST_ASSERT_FALSE(action_requires_value(CLAP_ACTION_STORE_CONST));
}

void test_action_requires_value_append_const(void) {
    TEST_ASSERT_FALSE(action_requires_value(CLAP_ACTION_APPEND_CONST));
}

void test_action_requires_value_count(void) {
    TEST_ASSERT_FALSE(action_requires_value(CLAP_ACTION_COUNT));
}

void test_action_requires_value_help(void) {
    TEST_ASSERT_FALSE(action_requires_value(CLAP_ACTION_HELP));
}

void test_action_requires_value_version(void) {
    TEST_ASSERT_FALSE(action_requires_value(CLAP_ACTION_VERSION));
}

/* ============================================================================
 * get_option_display_length Tests
 * ============================================================================ */

void test_get_option_display_length_basic(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--output");
    
    size_t len = get_option_display_length(arg);
    TEST_ASSERT_EQUAL(7, len);  /* "--output" */
    
    clap_parser_free(parser);
}

void test_get_option_display_length_with_short_option(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--output/-o");
    
    size_t len = get_option_display_length(arg);
    /* "--output, -o" */
    TEST_ASSERT_EQUAL(12, len);
    
    clap_parser_free(parser);
}

void test_get_option_display_length_with_metavar(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--output");
    clap_argument_metavar(arg, "FILE");
    
    size_t len = get_option_display_length(arg);
    TEST_ASSERT_EQUAL(12, len);  /* "--output FILE" */
    
    clap_parser_free(parser);
}

void test_get_option_display_length_with_choices(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--color");
    const char *choices[] = {"red", "green", "blue"};
    clap_argument_choices(arg, choices, 3);
    
    size_t len = get_option_display_length(arg);
    TEST_ASSERT_EQUAL(20, len);  /* "--color {red,green,blue}" */
    
    clap_parser_free(parser);
}

/* ============================================================================
 * clap_print_help Tests - Usage Line
 * ============================================================================ */

void test_print_help_usage_basic(void) {
    clap_parser_t *parser = clap_parser_new("prog", "Test description", NULL);
    FILE *capture = capture_stdout();
    
    clap_print_help(parser, capture);
    fflush(capture);
    
    const char *output = get_captured();
    TEST_ASSERT_NOT_NULL(strstr(output, "usage: prog [-h]"));
    
    clap_parser_free(parser);
}

void test_print_help_usage_with_positional(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_add_argument(parser, "input");
    clap_add_argument(parser, "output");
    
    FILE *capture = capture_stdout();
    clap_print_help(parser, capture);
    fflush(capture);
    
    const char *output = get_captured();
    TEST_ASSERT_NOT_NULL(strstr(output, "usage: prog [-h] input output"));
    
    clap_parser_free(parser);
}

void test_print_help_usage_with_optional(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--verbose");
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    
    FILE *capture = capture_stdout();
    clap_print_help(parser, capture);
    fflush(capture);
    
    const char *output = get_captured();
    TEST_ASSERT_NOT_NULL(strstr(output, "[--verbose]"));
    
    clap_parser_free(parser);
}

void test_print_help_usage_with_required_optional(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--host");
    clap_argument_required(arg, true);
    
    FILE *capture = capture_stdout();
    clap_print_help(parser, capture);
    fflush(capture);
    
    const char *output = get_captured();
    TEST_ASSERT_NOT_NULL(strstr(output, "--host HOST"));
    
    clap_parser_free(parser);
}

void test_print_help_usage_with_mutex_group(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group = clap_add_mutually_exclusive_group(parser, false);
    
    clap_argument_t *verbose = clap_add_argument(parser, "--verbose");
    clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);
    clap_argument_group(verbose, group);
    clap_mutex_group_add_argument(parser, group, verbose);
    
    clap_argument_t *quiet = clap_add_argument(parser, "--quiet");
    clap_argument_action(quiet, CLAP_ACTION_STORE_TRUE);
    clap_argument_group(quiet, group);
    clap_mutex_group_add_argument(parser, group, quiet);
    
    FILE *capture = capture_stdout();
    clap_print_help(parser, capture);
    fflush(capture);
    
    const char *output = get_captured();
    TEST_ASSERT_NOT_NULL(strstr(output, "[--verbose | --quiet]"));
    
    clap_parser_free(parser);
}

void test_print_help_usage_with_nargs(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *files = clap_add_argument(parser, "files");
    clap_argument_nargs(files, '*');
    
    FILE *capture = capture_stdout();
    clap_print_help(parser, capture);
    fflush(capture);
    
    const char *output = get_captured();
    TEST_ASSERT_NOT_NULL(strstr(output, "[files ...]"));
    
    clap_parser_free(parser);
}

void test_print_help_usage_with_subcommands(void) {
    clap_parser_t *parser = clap_parser_new("git", NULL, NULL);
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    clap_subparser_add(subparsers, "commit", NULL);
    clap_subparser_add(subparsers, "push", NULL);
    
    FILE *capture = capture_stdout();
    clap_print_help(parser, capture);
    fflush(capture);
    
    const char *output = get_captured();
    TEST_ASSERT_NOT_NULL(strstr(output, "{commit,push}"));
    
    clap_parser_free(parser);
}

/* ============================================================================
 * clap_print_help Tests - Sections
 * ============================================================================ */

void test_print_help_has_description(void) {
    clap_parser_t *parser = clap_parser_new("prog", "This is a test description", NULL);
    
    FILE *capture = capture_stdout();
    clap_print_help(parser, capture);
    fflush(capture);
    
    const char *output = get_captured();
    TEST_ASSERT_NOT_NULL(strstr(output, "This is a test description"));
    
    clap_parser_free(parser);
}

void test_print_help_has_positional_section(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_add_argument(parser, "input");
    
    FILE *capture = capture_stdout();
    clap_print_help(parser, capture);
    fflush(capture);
    
    const char *output = get_captured();
    TEST_ASSERT_NOT_NULL(strstr(output, "Positional arguments:"));
    TEST_ASSERT_NOT_NULL(strstr(output, "input"));
    
    clap_parser_free(parser);
}

void test_print_help_has_optional_section(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--verbose");
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    clap_argument_help(arg, "Enable verbose output");
    
    FILE *capture = capture_stdout();
    clap_print_help(parser, capture);
    fflush(capture);
    
    const char *output = get_captured();
    TEST_ASSERT_NOT_NULL(strstr(output, "Optional arguments:"));
    TEST_ASSERT_NOT_NULL(strstr(output, "--verbose"));
    TEST_ASSERT_NOT_NULL(strstr(output, "Enable verbose output"));
    
    clap_parser_free(parser);
}

void test_print_help_shows_default_value(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--output");
    clap_argument_default(arg, "stdout");
    clap_argument_help(arg, "Output file");
    
    FILE *capture = capture_stdout();
    clap_print_help(parser, capture);
    fflush(capture);
    
    const char *output = get_captured();
    TEST_ASSERT_NOT_NULL(strstr(output, "(default: stdout)"));
    
    clap_parser_free(parser);
}

void test_print_help_shows_choices(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--color");
    const char *choices[] = {"red", "green", "blue"};
    clap_argument_choices(arg, choices, 3);
    clap_argument_help(arg, "Choose a color");
    
    FILE *capture = capture_stdout();
    clap_print_help(parser, capture);
    fflush(capture);
    
    const char *output = get_captured();
    TEST_ASSERT_NOT_NULL(strstr(output, "{red,green,blue}"));
    
    clap_parser_free(parser);
}

void test_print_help_has_commands_section(void) {
    clap_parser_t *parser = clap_parser_new("git", NULL, NULL);
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    clap_subparser_add(subparsers, "commit", "Commit changes");
    
    FILE *capture = capture_stdout();
    clap_print_help(parser, capture);
    fflush(capture);
    
    const char *output = get_captured();
    TEST_ASSERT_NOT_NULL(strstr(output, "Commands:"));
    TEST_ASSERT_NOT_NULL(strstr(output, "commit"));
    TEST_ASSERT_NOT_NULL(strstr(output, "Commit changes"));
    
    clap_parser_free(parser);
}

void test_print_help_has_epilog(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, "This is the epilog");
    
    FILE *capture = capture_stdout();
    clap_print_help(parser, capture);
    fflush(capture);
    
    const char *output = get_captured();
    TEST_ASSERT_NOT_NULL(strstr(output, "This is the epilog"));
    
    clap_parser_free(parser);
}

void test_print_help_null_parser(void) {
    FILE *capture = capture_stdout();
    clap_print_help(NULL, capture);
    /* Should not crash */
    TEST_ASSERT_TRUE(1);
}

void test_print_help_null_stream(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_print_help(parser, NULL);
    /* Should not crash */
    TEST_ASSERT_TRUE(1);
    clap_parser_free(parser);
}

/* ============================================================================
 * clap_print_version Tests
 * ============================================================================ */

void test_print_version_with_set_version(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_parser_set_version(parser, "1.0.0");
    
    FILE *capture = capture_stdout();
    clap_print_version(parser, capture);
    fflush(capture);
    
    const char *output = get_captured();
    TEST_ASSERT_NOT_NULL(strstr(output, "prog version 1.0.0"));
    
    clap_parser_free(parser);
}

void test_print_version_without_set_version(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    
    FILE *capture = capture_stdout();
    clap_print_version(parser, capture);
    fflush(capture);
    
    const char *output = get_captured();
    TEST_ASSERT_NOT_NULL(strstr(output, "prog version unknown"));
    
    clap_parser_free(parser);
}

void test_print_version_null_parser(void) {
    FILE *capture = capture_stdout();
    clap_print_version(NULL, capture);
    /* Should not crash */
    TEST_ASSERT_TRUE(1);
}

void test_print_version_null_stream(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_print_version(parser, NULL);
    /* Should not crash */
    TEST_ASSERT_TRUE(1);
    clap_parser_free(parser);
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

void run_test_formatter(void) {
    /* wrap_text Tests */
    RUN_TEST(test_wrap_text_short_text);
    RUN_TEST(test_wrap_text_with_indent);
    RUN_TEST(test_wrap_text_long_text_wraps);
    RUN_TEST(test_wrap_text_first_line_already_indented);
    RUN_TEST(test_wrap_text_null_text);
    RUN_TEST(test_wrap_text_empty_text);
    
    /* get_upper_metavar Tests */
    RUN_TEST(test_get_upper_metavar_with_choices);
    RUN_TEST(test_get_upper_metavar_with_metavar);
    RUN_TEST(test_get_upper_metavar_with_dest);
    
    /* action_requires_value Tests */
    RUN_TEST(test_action_requires_value_store);
    RUN_TEST(test_action_requires_value_append);
    RUN_TEST(test_action_requires_value_store_true);
    RUN_TEST(test_action_requires_value_store_false);
    RUN_TEST(test_action_requires_value_store_const);
    RUN_TEST(test_action_requires_value_append_const);
    RUN_TEST(test_action_requires_value_count);
    RUN_TEST(test_action_requires_value_help);
    RUN_TEST(test_action_requires_value_version);
    
    /* get_option_display_length Tests */
    RUN_TEST(test_get_option_display_length_basic);
    RUN_TEST(test_get_option_display_length_with_short_option);
    RUN_TEST(test_get_option_display_length_with_metavar);
    RUN_TEST(test_get_option_display_length_with_choices);
    
    /* clap_print_help Usage Tests */
    RUN_TEST(test_print_help_usage_basic);
    RUN_TEST(test_print_help_usage_with_positional);
    RUN_TEST(test_print_help_usage_with_optional);
    RUN_TEST(test_print_help_usage_with_required_optional);
    RUN_TEST(test_print_help_usage_with_mutex_group);
    RUN_TEST(test_print_help_usage_with_nargs);
    RUN_TEST(test_print_help_usage_with_subcommands);
    
    /* clap_print_help Section Tests */
    RUN_TEST(test_print_help_has_description);
    RUN_TEST(test_print_help_has_positional_section);
    RUN_TEST(test_print_help_has_optional_section);
    RUN_TEST(test_print_help_shows_default_value);
    RUN_TEST(test_print_help_shows_choices);
    RUN_TEST(test_print_help_has_commands_section);
    RUN_TEST(test_print_help_has_epilog);
    RUN_TEST(test_print_help_null_parser);
    RUN_TEST(test_print_help_null_stream);
    
    /* clap_print_version Tests */
    RUN_TEST(test_print_version_with_set_version);
    RUN_TEST(test_print_version_without_set_version);
    RUN_TEST(test_print_version_null_parser);
    RUN_TEST(test_print_version_null_stream);
}

#ifdef STANDALONE_TEST
int main(void) {
    UNITY_BEGIN();
    run_test_formatter();
    return UNITY_END();
}
#endif