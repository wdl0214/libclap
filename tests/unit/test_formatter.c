/**
 * @file test_formatter.c
 * @brief Unit tests for clap_formatter.c
 */

/* Include source directly to access static functions */
#include "unity.h"
#include "../src/clap_formatter.c"

/* ============================================================================
 * Output Capture - Cross Platform (same as test_usage.c)
 * ============================================================================ */

static char g_captured[65536];
static size_t g_captured_len = 0;

#if defined(__APPLE__) || defined(__linux__) || defined(__unix__)
#include <unistd.h>

static int g_stdout_backup = -1;
static FILE *g_capture_file = NULL;

static void capture_start(void) {
    g_captured_len = 0;
    g_captured[0] = '\0';

    g_capture_file = tmpfile();
    if (!g_capture_file) return;

    fflush(stdout);
    g_stdout_backup = dup(STDOUT_FILENO);
    dup2(fileno(g_capture_file), STDOUT_FILENO);
}

static void capture_stop(void) {
    if (g_stdout_backup >= 0) {
        fflush(stdout);
        dup2(g_stdout_backup, STDOUT_FILENO);
        close(g_stdout_backup);
        g_stdout_backup = -1;
    }

    if (g_capture_file) {
        rewind(g_capture_file);
        g_captured_len = fread(g_captured, 1, sizeof(g_captured) - 1, g_capture_file);
        g_captured[g_captured_len] = '\0';
        fclose(g_capture_file);
        g_capture_file = NULL;
    }
}

#elif defined(_WIN32)
#include <io.h>

static int g_stdout_backup = -1;
static FILE *g_capture_file = NULL;

static void capture_start(void) {
    g_captured_len = 0;
    g_captured[0] = '\0';

    g_capture_file = tmpfile();
    if (!g_capture_file) return;

    fflush(stdout);
    g_stdout_backup = _dup(1);
    _dup2(_fileno(g_capture_file), 1);
}

static void capture_stop(void) {
    if (g_stdout_backup >= 0) {
        fflush(stdout);
        _dup2(g_stdout_backup, 1);
        _close(g_stdout_backup);
        g_stdout_backup = -1;
    }

    if (g_capture_file) {
        fseek(g_capture_file, 0, SEEK_SET);
        g_captured_len = fread(g_captured, 1, sizeof(g_captured) - 1, g_capture_file);
        g_captured[g_captured_len] = '\0';
        fclose(g_capture_file);
        g_capture_file = NULL;
    }
}

#else
/* Fallback - no capture */
static void capture_start(void) {
    g_captured_len = 0;
    g_captured[0] = '\0';
}

static void capture_stop(void) {
    /* Nothing */
}
#endif

static const char* get_captured(void) {
    return g_captured;
}

/* ============================================================================
 * Test Setup and Teardown
 * ============================================================================ */

void setUp(void) {
    capture_start();
}

void tearDown(void) {
    capture_stop();
}

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

static void extract_usage_line(const char *help_output, char *usage_line, size_t size) {
    const char *start = strstr(help_output, "usage: ");
    if (!start) {
        usage_line[0] = '\0';
        return;
    }

    const char *end = strchr(start, '\n');
    if (!end) {
        end = start + strlen(start);
    }

    size_t len = (size_t)(end - start);
    if (len >= size) len = size - 1;

    memcpy(usage_line, start, len);
    usage_line[len] = '\0';
}

static void assert_usage_contains(const char *expected) {
    const char *captured = get_captured();
    char usage[1024];
    extract_usage_line(captured, usage, sizeof(usage));

    if (strstr(usage, expected) == NULL) {
        printf("\n");
        printf("========================================\n");
        printf("USAGE MISMATCH\n");
        printf("========================================\n");
        printf("Expected to contain: %s\n", expected);
        printf("Actual usage: %s\n", usage);
        printf("----------------------------------------\n");
        printf("Full output:\n%s\n", captured);
        printf("========================================\n");
        TEST_FAIL();
    }
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

    for (int i = 0; i < 4; i++) {
        clap_buffer_cat(&buf, " ");
    }

    wrap_text(&buf, "Hello world", 4, 80, 1);

    TEST_ASSERT_EQUAL_STRING("    Hello world", clap_buffer_cstr(buf));
    clap_buffer_free(buf);
}

void test_wrap_text_long_text_wraps(void) {
    clap_buffer_t *buf = clap_buffer_empty();
    wrap_text(&buf, "This is a very long text that should wrap to multiple lines", 0, 20, 0);
    TEST_ASSERT_NOT_NULL(strchr(clap_buffer_cstr(buf), '\n'));
    clap_buffer_free(buf);
}

void test_wrap_text_first_line_already_indented(void) {
    clap_buffer_t *buf = clap_buffer_empty();
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
    TEST_ASSERT_EQUAL(15, len);

    clap_parser_free(parser);
}

void test_get_option_display_length_with_short_option(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--output/-o");
    size_t len = get_option_display_length(arg);
    TEST_ASSERT_EQUAL(19, len);  // "--output, -o output" = 19
    clap_parser_free(parser);
}

void test_get_option_display_length_with_metavar(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--output");
    clap_argument_metavar(arg, "FILE");

    size_t len = get_option_display_length(arg);
    TEST_ASSERT_EQUAL(13, len);

    clap_parser_free(parser);
}

void test_get_option_display_length_with_choices(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--color");
    const char *choices[] = {"red", "green", "blue"};
    clap_argument_choices(arg, choices, 3);
    size_t len = get_option_display_length(arg);
    TEST_ASSERT_EQUAL(24, len);  // "--color {red,green,blue}" = 7+1+16=24
    clap_parser_free(parser);
}

/* ============================================================================
 * clap_print_help Tests
 * ============================================================================ */

void test_print_help_usage_basic(void) {
    clap_parser_t *parser = clap_parser_new("prog", "Test description", NULL);

    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();

    assert_usage_contains("usage: prog [-h]");

    clap_parser_free(parser);
}

void test_print_help_usage_with_positional(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_add_argument(parser, "input");
    clap_add_argument(parser, "output");

    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();

    assert_usage_contains("input output");

    clap_parser_free(parser);
}

void test_print_help_usage_with_optional(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--verbose");
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);

    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();

    assert_usage_contains("[--verbose]");

    clap_parser_free(parser);
}

void test_print_help_usage_with_required_optional(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--host");
    clap_argument_required(arg, true);

    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();

    assert_usage_contains("--host HOST");

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

    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();

    assert_usage_contains("[--verbose | --quiet]");

    clap_parser_free(parser);
}

void test_print_help_usage_with_nargs(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *files = clap_add_argument(parser, "files");
    clap_argument_nargs(files, '*');

    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();

    assert_usage_contains("[files ...]");

    clap_parser_free(parser);
}

void test_print_help_usage_with_subcommands(void) {
    clap_parser_t *parser = clap_parser_new("git", NULL, NULL);
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    clap_subparser_add(subparsers, "commit", NULL);
    clap_subparser_add(subparsers, "push", NULL);

    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();

    assert_usage_contains("{commit,push}");

    clap_parser_free(parser);
}

void test_print_help_has_description(void) {
    clap_parser_t *parser = clap_parser_new("prog", "This is a test description", NULL);

    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();

    TEST_ASSERT_NOT_NULL(strstr(get_captured(), "This is a test description"));

    clap_parser_free(parser);
}

void test_print_help_has_optional_section(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--verbose");
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    clap_argument_help(arg, "Enable verbose output");

    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();

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

    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();

    TEST_ASSERT_NOT_NULL(strstr(get_captured(), "(default: stdout)"));

    clap_parser_free(parser);
}

void test_print_help_shows_choices(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--color");
    const char *choices[] = {"red", "green", "blue"};
    clap_argument_choices(arg, choices, 3);
    clap_argument_help(arg, "Choose a color");

    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();

    TEST_ASSERT_NOT_NULL(strstr(get_captured(), "{red,green,blue}"));

    clap_parser_free(parser);
}

void test_print_help_has_commands_section(void) {
    clap_parser_t *parser = clap_parser_new("git", NULL, NULL);
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    clap_subparser_add(subparsers, "commit", "Commit changes");

    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();

    const char *output = get_captured();
    TEST_ASSERT_NOT_NULL(strstr(output, "Commands:"));
    TEST_ASSERT_NOT_NULL(strstr(output, "commit"));
    TEST_ASSERT_NOT_NULL(strstr(output, "Commit changes"));

    clap_parser_free(parser);
}

void test_print_help_null_parser(void) {
    clap_print_help(NULL, stdout);
    TEST_ASSERT_TRUE(1);
}

void test_print_help_null_stream(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_print_help(parser, NULL);
    TEST_ASSERT_TRUE(1);
    clap_parser_free(parser);
}

/* ============================================================================
 * clap_print_version Tests
 * ============================================================================ */

void test_print_version_with_set_version(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_parser_set_version(parser, "1.0.0");

    clap_print_version(parser, stdout);
    fflush(stdout);
    capture_stop();

    TEST_ASSERT_NOT_NULL(strstr(get_captured(), "prog version 1.0.0"));

    clap_parser_free(parser);
}

void test_print_version_without_set_version(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);

    clap_print_version(parser, stdout);
    fflush(stdout);
    capture_stop();

    TEST_ASSERT_NOT_NULL(strstr(get_captured(), "prog version unknown"));

    clap_parser_free(parser);
}

void test_print_version_null_parser(void) {
    clap_print_version(NULL, stdout);
    TEST_ASSERT_TRUE(1);
}

void test_print_version_null_stream(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_print_version(parser, NULL);
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

    /* clap_print_help Tests */
    RUN_TEST(test_print_help_usage_basic);
    RUN_TEST(test_print_help_usage_with_positional);
    RUN_TEST(test_print_help_usage_with_optional);
    RUN_TEST(test_print_help_usage_with_required_optional);
    RUN_TEST(test_print_help_usage_with_mutex_group);
    RUN_TEST(test_print_help_usage_with_nargs);
    RUN_TEST(test_print_help_usage_with_subcommands);
    RUN_TEST(test_print_help_has_description);
    RUN_TEST(test_print_help_has_optional_section);
    RUN_TEST(test_print_help_shows_default_value);
    RUN_TEST(test_print_help_shows_choices);
    RUN_TEST(test_print_help_has_commands_section);
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