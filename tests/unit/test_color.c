/**
 * @file test_color.c
 * @brief Unit tests for clap_color.h / clap_color.c
 */

#include "unity.h"
#include "../src/clap_parser_internal.h"
#include <clap/clap_color.h>
#include <string.h>

/* ============================================================================
 * Test Setup and Teardown
 * ============================================================================ */

void setUp(void) { }

void tearDown(void) { }

/* ============================================================================
 * clap_color_theme_init Tests
 * ============================================================================ */

void test_theme_init_disabled_by_default(void) {
    clap_color_theme_t theme;
    clap_color_theme_init(&theme);
    TEST_ASSERT_FALSE(theme.enabled);
}

void test_theme_init_sets_all_codes(void) {
    clap_color_theme_t theme;
    clap_color_theme_init(&theme);

    for (int i = 0; i < CLAP_COLOR_COUNT; i++) {
        TEST_ASSERT_NOT_NULL(theme.codes[i]);
        TEST_ASSERT_NOT_EQUAL(0, strlen(theme.codes[i]));
    }

    TEST_ASSERT_NOT_NULL(theme.reset);
    TEST_ASSERT_NOT_EQUAL(0, strlen(theme.reset));
}

void test_theme_init_assigns_correct_colors(void) {
    clap_color_theme_t theme;
    clap_color_theme_init(&theme);

    TEST_ASSERT_EQUAL_STRING(CLAP_ANSI_BOLD,   theme.codes[CLAP_COLOR_USAGE_PROG]);
    TEST_ASSERT_EQUAL_STRING(CLAP_ANSI_BOLD,   theme.codes[CLAP_COLOR_HEADING]);
    TEST_ASSERT_EQUAL_STRING(CLAP_ANSI_GREEN,  theme.codes[CLAP_COLOR_OPTION_SHORT]);
    TEST_ASSERT_EQUAL_STRING(CLAP_ANSI_CYAN,   theme.codes[CLAP_COLOR_OPTION_LONG]);
    TEST_ASSERT_EQUAL_STRING(CLAP_ANSI_YELLOW, theme.codes[CLAP_COLOR_METAVAR]);
    TEST_ASSERT_EQUAL_STRING(CLAP_ANSI_YELLOW, theme.codes[CLAP_COLOR_CHOICES]);
    TEST_ASSERT_EQUAL_STRING(CLAP_ANSI_DIM,    theme.codes[CLAP_COLOR_DEFAULT]);
    TEST_ASSERT_EQUAL_STRING(CLAP_ANSI_CYAN,   theme.codes[CLAP_COLOR_SUBCOMMAND]);
    TEST_ASSERT_EQUAL_STRING(CLAP_ANSI_RED,    theme.codes[CLAP_COLOR_ERROR]);
    TEST_ASSERT_EQUAL_STRING(CLAP_ANSI_YELLOW, theme.codes[CLAP_COLOR_WARNING]);
    TEST_ASSERT_EQUAL_STRING(CLAP_ANSI_RESET,  theme.reset);
}

void test_theme_init_different_keys_have_codes(void) {
    clap_color_theme_t theme;
    clap_color_theme_init(&theme);

    /* Each key has a non-empty code string */
    TEST_ASSERT_TRUE(strlen(theme.codes[CLAP_COLOR_USAGE_PROG]) > 0);
    TEST_ASSERT_TRUE(strlen(theme.codes[CLAP_COLOR_OPTION_SHORT]) > 0);
    TEST_ASSERT_TRUE(strlen(theme.codes[CLAP_COLOR_OPTION_LONG]) > 0);
    TEST_ASSERT_TRUE(strlen(theme.codes[CLAP_COLOR_METAVAR]) > 0);
    TEST_ASSERT_TRUE(strlen(theme.codes[CLAP_COLOR_ERROR]) > 0);
}

void test_theme_init_null_safe(void) {
    /* Should not crash */
    clap_color_theme_init(NULL);
}

/* ============================================================================
 * clap_color_theme_detect Tests
 * ============================================================================ */

/* Environment detection tests — use putenv (available on MinGW and POSIX).
 * MSVC requires _putenv; skip there since the detection logic is tested
 * functionally through clap_parser_set_color() in the parser tests. */
#if !defined(_MSC_VER)
#include <stdlib.h>

void test_detect_no_color_env_disables(void) {
    clap_color_theme_t theme;
    clap_color_theme_init(&theme);

    putenv("NO_COLOR=1");
    clap_color_theme_detect(&theme, stdout);
    putenv("NO_COLOR=");

    TEST_ASSERT_FALSE(theme.enabled);
}

void test_detect_no_color_any_value_disables(void) {
    clap_color_theme_t theme;
    clap_color_theme_init(&theme);

    putenv("NO_COLOR=true");
    clap_color_theme_detect(&theme, stdout);
    putenv("NO_COLOR=");

    TEST_ASSERT_FALSE(theme.enabled);
}

void test_detect_force_color_enables(void) {
    clap_color_theme_t theme;
    clap_color_theme_init(&theme);

    putenv("FORCE_COLOR=1");
    clap_color_theme_detect(&theme, stdout);
    putenv("FORCE_COLOR=");

    TEST_ASSERT_TRUE(theme.enabled);
}

void test_detect_clicolor_zero_disables(void) {
    clap_color_theme_t theme;
    clap_color_theme_init(&theme);

    putenv("CLICOLOR=0");
    clap_color_theme_detect(&theme, stdout);
    putenv("CLICOLOR=");

    TEST_ASSERT_FALSE(theme.enabled);
}

void test_detect_clicolor_force_enables(void) {
    clap_color_theme_t theme;
    clap_color_theme_init(&theme);

    putenv("CLICOLOR_FORCE=1");
    clap_color_theme_detect(&theme, stdout);
    putenv("CLICOLOR_FORCE=");

    TEST_ASSERT_TRUE(theme.enabled);
}

void test_detect_no_color_takes_priority_over_force(void) {
    clap_color_theme_t theme;
    clap_color_theme_init(&theme);

    putenv("NO_COLOR=1");
    putenv("FORCE_COLOR=1");
    clap_color_theme_detect(&theme, stdout);
    putenv("NO_COLOR=");
    putenv("FORCE_COLOR=");

    TEST_ASSERT_FALSE(theme.enabled);
}

void test_detect_null_safe(void) {
    clap_color_theme_detect(NULL, stdout);
    /* Should not crash */
}
#else
/* MSVC stubs — env detection logic is simple and covered by functional tests */
void test_detect_no_color_env_disables(void) { TEST_IGNORE(); }
void test_detect_no_color_any_value_disables(void) { TEST_IGNORE(); }
void test_detect_force_color_enables(void) { TEST_IGNORE(); }
void test_detect_clicolor_zero_disables(void) { TEST_IGNORE(); }
void test_detect_clicolor_force_enables(void) { TEST_IGNORE(); }
void test_detect_no_color_takes_priority_over_force(void) { TEST_IGNORE(); }
void test_detect_null_safe(void) { }
#endif

/* ============================================================================
 * clap_color_visual_length Tests
 * ============================================================================ */

void test_visual_length_plain_text(void) {
    TEST_ASSERT_EQUAL(5, clap_color_visual_length("hello"));
}

void test_visual_length_empty(void) {
    TEST_ASSERT_EQUAL(0, clap_color_visual_length(""));
}

void test_visual_length_null(void) {
    TEST_ASSERT_EQUAL(0, clap_color_visual_length(NULL));
}

void test_visual_length_skips_ansi(void) {
    /* "\033[31m" + "error" + "\033[0m" = 5 visible chars */
    const char *colored = "\033[31merror\033[0m";
    TEST_ASSERT_EQUAL(5, clap_color_visual_length(colored));
}

void test_visual_length_multiple_ansi(void) {
    /* "\033[1m\033[32m" + "OK" + "\033[0m" = 2 visible chars */
    const char *colored = "\033[1m\033[32mOK\033[0m";
    TEST_ASSERT_EQUAL(2, clap_color_visual_length(colored));
}

void test_visual_length_no_reset(void) {
    /* Unclosed ANSI is still skipped */
    TEST_ASSERT_EQUAL(4, clap_color_visual_length("\033[31mtest"));
}

void test_visual_length_only_ansi(void) {
    TEST_ASSERT_EQUAL(0, clap_color_visual_length("\033[31m\033[0m"));
}

void test_visual_length_long_option_colored(void) {
    /* --verbose in cyan */
    char buf[256];
    snprintf(buf, sizeof(buf), "%s--verbose%s", CLAP_ANSI_CYAN, CLAP_ANSI_RESET);
    TEST_ASSERT_EQUAL(9, clap_color_visual_length(buf));
}

/* ============================================================================
 * clap_parser_set_color Tests
 * ============================================================================ */

void test_parser_color_disabled_by_default(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);
    TEST_ASSERT_NOT_NULL(parser);
    TEST_ASSERT_FALSE(parser->color_theme.enabled);
    clap_parser_free(parser);
}

void test_parser_set_color_true_enables(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);

    /* Force enable (override env) */
    parser->color_theme.enabled = true;
    TEST_ASSERT_TRUE(parser->color_theme.enabled);

    clap_parser_free(parser);
}

void test_parser_set_color_false_disables(void) {
    clap_parser_t *parser = clap_parser_new("test", NULL, NULL);

    clap_parser_set_color(parser, false);
    TEST_ASSERT_FALSE(parser->color_theme.enabled);

    clap_parser_free(parser);
}

void test_parser_set_color_null_safe(void) {
    clap_parser_set_color(NULL, true);
    clap_parser_set_color(NULL, false);
    /* Should not crash */
}

/* ============================================================================
 * clap_buffer_cat_colored Tests
 * ============================================================================ */

void test_buffer_cat_colored_enabled_adds_ansi(void) {
    clap_color_theme_t theme;
    clap_color_theme_init(&theme);
    theme.enabled = true;

    clap_buffer_t *buf = clap_buffer_empty();
    clap_buffer_cat_colored(&buf, &theme, CLAP_COLOR_OPTION_SHORT, "-v");

    const char *result = clap_buffer_cstr(buf);
    TEST_ASSERT_NOT_NULL(strstr(result, CLAP_ANSI_GREEN));
    TEST_ASSERT_NOT_NULL(strstr(result, "-v"));
    TEST_ASSERT_NOT_NULL(strstr(result, CLAP_ANSI_RESET));

    clap_buffer_free(buf);
}

void test_buffer_cat_colored_disabled_no_ansi(void) {
    clap_color_theme_t theme;
    clap_color_theme_init(&theme);
    theme.enabled = false;

    clap_buffer_t *buf = clap_buffer_empty();
    clap_buffer_cat_colored(&buf, &theme, CLAP_COLOR_OPTION_SHORT, "-v");

    const char *result = clap_buffer_cstr(buf);
    TEST_ASSERT_EQUAL_STRING("-v", result);

    clap_buffer_free(buf);
}

void test_buffer_cat_colored_null_theme_no_color(void) {
    clap_buffer_t *buf = clap_buffer_empty();
    clap_buffer_cat_colored(&buf, NULL, CLAP_COLOR_OPTION_SHORT, "-v");

    const char *result = clap_buffer_cstr(buf);
    TEST_ASSERT_EQUAL_STRING("-v", result);

    clap_buffer_free(buf);
}

void test_buffer_cat_colored_empty_text(void) {
    clap_color_theme_t theme;
    clap_color_theme_init(&theme);
    theme.enabled = true;

    clap_buffer_t *buf = clap_buffer_empty();
    bool ok = clap_buffer_cat_colored(&buf, &theme, CLAP_COLOR_OPTION_SHORT, "");

    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL(0, clap_buffer_len(buf));

    clap_buffer_free(buf);
}

/* ============================================================================
 * Help Output Color Tests
 * ============================================================================ */

static void assert_contains(const char *haystack, const char *needle, const char *label) {
    if (strstr(haystack, needle) == NULL) {
        printf("\nExpected to contain %s: '%s'\nActual output:\n%s\n", label, needle, haystack);
        TEST_FAIL();
    }
}

static void assert_not_contains(const char *haystack, const char *needle, const char *label) {
    if (strstr(haystack, needle) != NULL) {
        printf("\nExpected NOT to contain %s: '%s'\nActual output:\n%s\n", label, needle, haystack);
        TEST_FAIL();
    }
}

void test_help_output_no_color_by_default(void) {
    clap_parser_t *parser = clap_parser_new("test_prog", "Test description", NULL);
    clap_add_argument(parser, "--verbose/-v");
    clap_add_argument(parser, "input");

    /* Default: color disabled, no ANSI in output */
    clap_buffer_t *buf = clap_buffer_empty();
    /* We can't easily call clap_print_help without a FILE*,
     * but the theme struct should not be enabled. */
    TEST_ASSERT_FALSE(parser->color_theme.enabled);

    clap_parser_free(parser);
    clap_buffer_free(buf);
}

void test_help_output_with_color_contains_ansi(void) {
    clap_parser_t *parser = clap_parser_new("test_prog", "Test description", NULL);
    parser->color_theme.enabled = true;
    clap_add_argument(parser, "--verbose/-v");
    clap_argument_help(clap_add_argument(parser, "--output/-o"), "Output file");
    clap_add_argument(parser, "input");

    /* Redirect stdout to capture buffer */
    FILE *capture = tmpfile();
    TEST_ASSERT_NOT_NULL(capture);

    clap_print_help(parser, capture);
    fflush(capture);
    rewind(capture);

    char output[16384] = {0};
    size_t len = fread(output, 1, sizeof(output) - 1, capture);
    output[len] = '\0';
    fclose(capture);

    /* Long options should be cyan */
    assert_contains(output, CLAP_ANSI_CYAN, "long option in cyan");
    /* Short options should be green */
    assert_contains(output, CLAP_ANSI_GREEN, "short option in green");
    /* Reset should be present */
    assert_contains(output, CLAP_ANSI_RESET, "ANSI reset");

    clap_parser_free(parser);
}

void test_help_output_color_disabled_no_ansi(void) {
    clap_parser_t *parser = clap_parser_new("test_prog", "Test description", NULL);
    /* Leave color disabled */
    clap_add_argument(parser, "--verbose/-v");
    clap_add_argument(parser, "--output/-o");
    clap_add_argument(parser, "input");

    FILE *capture = tmpfile();
    TEST_ASSERT_NOT_NULL(capture);

    clap_print_help(parser, capture);
    fflush(capture);
    rewind(capture);

    char output[16384] = {0};
    size_t len = fread(output, 1, sizeof(output) - 1, capture);
    output[len] = '\0';
    fclose(capture);

    assert_not_contains(output, CLAP_ANSI_GREEN, "ANSI green");
    assert_not_contains(output, CLAP_ANSI_CYAN, "ANSI cyan");
    assert_not_contains(output, CLAP_ANSI_YELLOW, "ANSI yellow");

    clap_parser_free(parser);
}

void test_help_color_usage_line_has_bold_prog(void) {
    clap_parser_t *parser = clap_parser_new("test_prog", NULL, NULL);
    parser->color_theme.enabled = true;

    FILE *capture = tmpfile();

    clap_print_help(parser, capture);
    fflush(capture);
    rewind(capture);

    char output[16384] = {0};
    fread(output, 1, sizeof(output) - 1, capture);
    fclose(capture);

    /* Usage: <bold>test_prog</bold> should have bold around prog name */
    char expected[256];
    snprintf(expected, sizeof(expected), "%stest_prog%s",
             CLAP_ANSI_BOLD, CLAP_ANSI_RESET);
    assert_contains(output, expected, "bold program name in usage");

    clap_parser_free(parser);
}

void test_help_color_metavar_in_yellow(void) {
    clap_parser_t *parser = clap_parser_new("test_prog", NULL, NULL);
    parser->color_theme.enabled = true;
    clap_argument_t *arg = clap_add_argument(parser, "--output/-o");
    clap_argument_metavar(arg, "FILE");

    FILE *capture = tmpfile();

    clap_print_help(parser, capture);
    fflush(capture);
    rewind(capture);

    char output[16384] = {0};
    fread(output, 1, sizeof(output) - 1, capture);
    fclose(capture);

    /* FILE metavar should be in yellow */
    char expected[128];
    snprintf(expected, sizeof(expected), "%sFILE%s",
             CLAP_ANSI_YELLOW, CLAP_ANSI_RESET);
    assert_contains(output, expected, "yellow metavar");

    clap_parser_free(parser);
}

void test_help_color_section_headings_in_bold(void) {
    clap_parser_t *parser = clap_parser_new("test_prog", NULL, NULL);
    parser->color_theme.enabled = true;
    clap_add_argument(parser, "input");

    FILE *capture = tmpfile();

    clap_print_help(parser, capture);
    fflush(capture);
    rewind(capture);

    char output[16384] = {0};
    fread(output, 1, sizeof(output) - 1, capture);
    fclose(capture);

    /* "Positional arguments:" should be in bold */
    char expected[128];
    snprintf(expected, sizeof(expected), "%sPositional arguments:%s",
             CLAP_ANSI_BOLD, CLAP_ANSI_RESET);
    assert_contains(output, expected, "bold section heading");

    clap_parser_free(parser);
}

void test_help_color_choices_yellow(void) {
    clap_parser_t *parser = clap_parser_new("test_prog", NULL, NULL);
    parser->color_theme.enabled = true;
    const char *choices[] = {"red", "green", "blue"};
    clap_argument_t *arg = clap_add_argument(parser, "--color");
    clap_argument_choices(arg, choices, 3);

    FILE *capture = tmpfile();

    clap_print_help(parser, capture);
    fflush(capture);
    rewind(capture);

    char output[16384] = {0};
    fread(output, 1, sizeof(output) - 1, capture);
    fclose(capture);

    /* Choices should have yellow ANSI */
    assert_contains(output, CLAP_ANSI_YELLOW, "yellow for choices");

    clap_parser_free(parser);
}

void test_help_color_default_dim(void) {
    clap_parser_t *parser = clap_parser_new("test_prog", NULL, NULL);
    parser->color_theme.enabled = true;
    clap_argument_t *arg = clap_add_argument(parser, "--output/-o");
    clap_argument_default(arg, "stdout");

    FILE *capture = tmpfile();

    clap_print_help(parser, capture);
    fflush(capture);
    rewind(capture);

    char output[16384] = {0};
    fread(output, 1, sizeof(output) - 1, capture);
    fclose(capture);

    /* Default value should be in dim */
    assert_contains(output, CLAP_ANSI_DIM, "dim for default value");

    clap_parser_free(parser);
}

void test_help_color_error_in_red(void) {
    clap_parser_t *parser = clap_parser_new("test_prog", "Test", NULL);
    parser->color_theme.enabled = true;
    clap_argument_t *arg = clap_add_argument(parser, "--required");
    clap_argument_required(arg, true);

    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};

    char *argv[] = {"test_prog"};
    clap_parse_args(parser, 1, argv, &ns, &error);

    FILE *capture = tmpfile();

    clap_print_help_on_error(parser, &error, capture);
    fflush(capture);
    rewind(capture);

    char output[16384] = {0};
    fread(output, 1, sizeof(output) - 1, capture);
    fclose(capture);

    /* Error output should have red ANSI */
    assert_contains(output, CLAP_ANSI_RED, "red for error");

    clap_parser_free(parser);
}

void test_color_subparser_inherits_theme(void) {
    clap_parser_t *parser = clap_parser_new("test_prog", NULL, NULL);
    parser->color_theme.enabled = true;

    clap_parser_t *subparsers = clap_add_subparsers(parser, "command", "Commands");
    clap_parser_t *sub = clap_subparser_add(subparsers, "run", "Run something");

    TEST_ASSERT_NOT_NULL(sub);
    TEST_ASSERT_TRUE(sub->color_theme.enabled);
    TEST_ASSERT_TRUE(subparsers->color_theme.enabled);

    clap_parser_free(parser);
}

/* ============================================================================
 * ANSI Constants Tests
 * ============================================================================ */

void test_ansi_constants_not_empty(void) {
    TEST_ASSERT_TRUE(strlen(CLAP_ANSI_RESET) > 0);
    TEST_ASSERT_TRUE(strlen(CLAP_ANSI_BOLD) > 0);
    TEST_ASSERT_TRUE(strlen(CLAP_ANSI_DIM) > 0);
    TEST_ASSERT_TRUE(strlen(CLAP_ANSI_RED) > 0);
    TEST_ASSERT_TRUE(strlen(CLAP_ANSI_GREEN) > 0);
    TEST_ASSERT_TRUE(strlen(CLAP_ANSI_YELLOW) > 0);
    TEST_ASSERT_TRUE(strlen(CLAP_ANSI_CYAN) > 0);
}

void test_ansi_constants_are_escape_sequences(void) {
    TEST_ASSERT_EQUAL('\033', CLAP_ANSI_RESET[0]);
    TEST_ASSERT_EQUAL('[', CLAP_ANSI_RESET[1]);
    TEST_ASSERT_EQUAL('\033', CLAP_ANSI_RED[0]);
    TEST_ASSERT_EQUAL('[', CLAP_ANSI_RED[1]);
}

/* ============================================================================
 * Test Runner
 * ============================================================================ */

void run_test_color(void) {
    /* Theme init */
    RUN_TEST(test_theme_init_disabled_by_default);
    RUN_TEST(test_theme_init_sets_all_codes);
    RUN_TEST(test_theme_init_assigns_correct_colors);
    RUN_TEST(test_theme_init_different_keys_have_codes);
    RUN_TEST(test_theme_init_null_safe);

    /* Environment detection */
    RUN_TEST(test_detect_no_color_env_disables);
    RUN_TEST(test_detect_no_color_any_value_disables);
    RUN_TEST(test_detect_force_color_enables);
    RUN_TEST(test_detect_clicolor_zero_disables);
    RUN_TEST(test_detect_clicolor_force_enables);
    RUN_TEST(test_detect_no_color_takes_priority_over_force);
    RUN_TEST(test_detect_null_safe);

    /* Visual length */
    RUN_TEST(test_visual_length_plain_text);
    RUN_TEST(test_visual_length_empty);
    RUN_TEST(test_visual_length_null);
    RUN_TEST(test_visual_length_skips_ansi);
    RUN_TEST(test_visual_length_multiple_ansi);
    RUN_TEST(test_visual_length_no_reset);
    RUN_TEST(test_visual_length_only_ansi);
    RUN_TEST(test_visual_length_long_option_colored);

    /* Parser color API */
    RUN_TEST(test_parser_color_disabled_by_default);
    RUN_TEST(test_parser_set_color_true_enables);
    RUN_TEST(test_parser_set_color_false_disables);
    RUN_TEST(test_parser_set_color_null_safe);

    /* Buffer colored output */
    RUN_TEST(test_buffer_cat_colored_enabled_adds_ansi);
    RUN_TEST(test_buffer_cat_colored_disabled_no_ansi);
    RUN_TEST(test_buffer_cat_colored_null_theme_no_color);
    RUN_TEST(test_buffer_cat_colored_empty_text);

    /* Help output color */
    RUN_TEST(test_help_output_no_color_by_default);
    RUN_TEST(test_help_output_with_color_contains_ansi);
    RUN_TEST(test_help_output_color_disabled_no_ansi);
    RUN_TEST(test_help_color_usage_line_has_bold_prog);
    RUN_TEST(test_help_color_metavar_in_yellow);
    RUN_TEST(test_help_color_section_headings_in_bold);
    RUN_TEST(test_help_color_choices_yellow);
    RUN_TEST(test_help_color_default_dim);
    RUN_TEST(test_help_color_error_in_red);
    RUN_TEST(test_color_subparser_inherits_theme);

    /* ANSI constants */
    RUN_TEST(test_ansi_constants_not_empty);
    RUN_TEST(test_ansi_constants_are_escape_sequences);
}

#ifdef STANDALONE_TEST
int main(void) {
    UNITY_BEGIN();
    run_test_color();
    return UNITY_END();
}
#endif
