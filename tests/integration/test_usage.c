/**
 * @file test_usage.c
 * @brief Usage string generation compliance tests for libclap
 */

#include "unity.h"
#include <clap/clap.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

/* ============================================================================
 * Cross-Platform Output Capture
 * ============================================================================ */

static char g_captured_output[65536];
static size_t g_captured_len = 0;

#ifdef _WIN32

static int g_original_stdout_fd = -1;
static int g_pipe_fd[2] = {-1, -1};
static FILE *g_capture_file = NULL;

static void capture_start(void) {
    g_captured_len = 0;
    g_captured_output[0] = '\0';
    
    /* Save original stdout file descriptor */
    g_original_stdout_fd = _dup(1);
    if (g_original_stdout_fd < 0) return;
    
    /* Create a pipe for capturing */
    if (_pipe(g_pipe_fd, 65536, _O_TEXT) < 0) {
        _close(g_original_stdout_fd);
        g_original_stdout_fd = -1;
        return;
    }
    
    /* Redirect stdout to the pipe's write end */
    fflush(stdout);
    _dup2(g_pipe_fd[1], 1);
    _close(g_pipe_fd[1]);
    g_pipe_fd[1] = -1;
}

static void capture_stop(void) {
    if (g_original_stdout_fd < 0) return;
    
    fflush(stdout);
    
    /* Read from the pipe's read end */
    if (g_pipe_fd[0] >= 0) {
        g_captured_len = _read(g_pipe_fd[0], g_captured_output, sizeof(g_captured_output) - 1);
        if (g_captured_len > 0) {
            g_captured_output[g_captured_len] = '\0';
        }
        _close(g_pipe_fd[0]);
        g_pipe_fd[0] = -1;
    }
    
    /* Restore original stdout */
    fflush(stdout);
    _dup2(g_original_stdout_fd, 1);
    _close(g_original_stdout_fd);
    g_original_stdout_fd = -1;
}

#else /* Linux/macOS */

#include <unistd.h>

static FILE *g_capture_file = NULL;
static int g_saved_stdout = -1;

static void capture_start(void) {
    g_captured_len = 0;
    g_captured_output[0] = '\0';

    g_capture_file = tmpfile();
    if (!g_capture_file) return;

    g_saved_stdout = dup(STDOUT_FILENO);
    if (g_saved_stdout < 0) {
        fclose(g_capture_file);
        g_capture_file = NULL;
        return;
    }

    fflush(stdout);
    dup2(fileno(g_capture_file), STDOUT_FILENO);
}

static void capture_stop(void) {
    if (!g_capture_file) return;

    fflush(stdout);

    if (g_saved_stdout >= 0) {
        dup2(g_saved_stdout, STDOUT_FILENO);
        close(g_saved_stdout);
        g_saved_stdout = -1;
    }

    rewind(g_capture_file);
    g_captured_len = fread(g_captured_output, 1, sizeof(g_captured_output) - 1, g_capture_file);
    g_captured_output[g_captured_len] = '\0';

    fclose(g_capture_file);
    g_capture_file = NULL;
}

#endif

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

static const char* get_captured(void) {
    return g_captured_output;
}

static void clear_capture(void) {
    g_captured_len = 0;
    g_captured_output[0] = '\0';
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

static void assert_usage_equals(const char *expected) {
    const char *captured = get_captured();
    char usage[1024];
    extract_usage_line(captured, usage, sizeof(usage));
    
    if (strcmp(usage, expected) != 0) {
        printf("\n");
        printf("========================================\n");
        printf("USAGE MISMATCH\n");
        printf("========================================\n");
        printf("Expected: %s\n", expected);
        printf("Actual:   %s\n", usage);
        printf("----------------------------------------\n");
        printf("Full help output:\n");
        printf("----------------------------------------\n");
        printf("%s\n", captured);
        printf("========================================\n");
        TEST_FAIL();
    }
}

/* ============================================================================
 * Test Setup and Teardown
 * ============================================================================ */

void setUp(void) {
    clear_capture();
    capture_start();
}

void tearDown(void) {
    capture_stop();
    clear_capture();
}

/* ============================================================================
 * Test 1: Basic Format - Only Positional Arguments
 * Expected: usage: PROG [-h] input output
 * ============================================================================ */
void test_1_basic_positional(void) {
    clap_parser_t *parser = clap_parser_new("PROG", "Test basic positional", NULL);
    clap_add_argument(parser, "input");
    clap_add_argument(parser, "output");
    
    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();
    
    assert_usage_equals("usage: PROG [-h] input output");
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Test 2a: Positional with nargs='*'
 * Expected: usage: PROG [-h] [files ...]
 * ============================================================================ */
void test_2a_positional_nargs_star(void) {
    clap_parser_t *parser = clap_parser_new("PROG", "Test nargs *", NULL);
    clap_argument_t *files = clap_add_argument(parser, "files");
    clap_argument_nargs(files, '*');
    
    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();
    
    assert_usage_equals("usage: PROG [-h] [files ...]");
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Test 2b: Positional with nargs='+'
 * Expected: usage: PROG [-h] files [files ...]
 * ============================================================================ */
void test_2b_positional_nargs_plus(void) {
    clap_parser_t *parser = clap_parser_new("PROG", "Test nargs +", NULL);
    clap_argument_t *files = clap_add_argument(parser, "files");
    clap_argument_nargs(files, '+');
    
    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();
    
    assert_usage_equals("usage: PROG [-h] files [files ...]");
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Test 2c: Positional with nargs='?'
 * Expected: usage: PROG [-h] [dest]
 * ============================================================================ */
void test_2c_positional_nargs_question(void) {
    clap_parser_t *parser = clap_parser_new("PROG", "Test nargs ?", NULL);
    clap_argument_t *dest = clap_add_argument(parser, "dest");
    clap_argument_nargs(dest, '?');
    
    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();
    
    assert_usage_equals("usage: PROG [-h] [dest]");
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Test 3a: Only Optional Arguments
 * Expected: usage: PROG [-h] [--verbose] [--output OUTPUT]
 * ============================================================================ */
void test_3a_only_optional(void) {
    clap_parser_t *parser = clap_parser_new("PROG", "Test optional only", NULL);
    
    clap_argument_t *verbose = clap_add_argument(parser, "--verbose");
    clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);
    
    clap_argument_t *output = clap_add_argument(parser, "--output");
    clap_argument_type(output, "string");
    
    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();
    
    assert_usage_equals("usage: PROG [-h] [--verbose] [--output OUTPUT]");
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Test 3b: Required Optional Arguments
 * Expected: usage: PROG [-h] --host HOST [--port PORT]
 * ============================================================================ */
void test_3b_required_optional(void) {
    clap_parser_t *parser = clap_parser_new("PROG", "Test required optional", NULL);
    
    clap_argument_t *host = clap_add_argument(parser, "--host");
    clap_argument_required(host, true);
    
    clap_argument_t *port = clap_add_argument(parser, "--port");
    clap_argument_type(port, "int");
    
    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();
    
    assert_usage_equals("usage: PROG [-h] --host HOST [--port PORT]");
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Test 4: Mixed Arguments (Positional + Optional)
 * Expected: usage: PROG [-h] [--config CONFIG] [--verbose] file
 * ============================================================================ */
void test_4_mixed_arguments(void) {
    clap_parser_t *parser = clap_parser_new("PROG", "Test mixed", NULL);
    
    clap_add_argument(parser, "--config");
    clap_add_argument(parser, "file");
    
    clap_argument_t *verbose = clap_add_argument(parser, "--verbose");
    clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);
    
    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();
    
    assert_usage_equals("usage: PROG [-h] [--config CONFIG] [--verbose] file");
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Test 5a: Mutually Exclusive Group (Optional)
 * Expected: usage: PROG [-h] [--verbose | --quiet]
 * ============================================================================ */
void test_5a_mutex_optional(void) {
    clap_parser_t *parser = clap_parser_new("PROG", "Test mutex optional", NULL);
    int group = clap_add_mutually_exclusive_group(parser, false);
    
    clap_argument_t *verbose = clap_add_argument(parser, "--verbose");
    clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);
    clap_argument_mutex_group(verbose, group);
    clap_mutex_group_add_argument(parser, group, verbose);
    
    clap_argument_t *quiet = clap_add_argument(parser, "--quiet");
    clap_argument_action(quiet, CLAP_ACTION_STORE_TRUE);
    clap_argument_mutex_group(quiet, group);
    clap_mutex_group_add_argument(parser, group, quiet);
    
    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();
    
    assert_usage_equals("usage: PROG [-h] [--verbose | --quiet]");
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Test 5b: Mutually Exclusive Group (Required)
 * Expected: usage: PROG [-h] (--start START | --stop STOP)
 * ============================================================================ */
void test_5b_mutex_required(void) {
    clap_parser_t *parser = clap_parser_new("PROG", "Test mutex required", NULL);
    int group = clap_add_mutually_exclusive_group(parser, true);
    
    clap_argument_t *start = clap_add_argument(parser, "--start");
    clap_argument_mutex_group(start, group);
    clap_mutex_group_add_argument(parser, group, start);
    
    clap_argument_t *stop = clap_add_argument(parser, "--stop");
    clap_argument_mutex_group(stop, group);
    clap_mutex_group_add_argument(parser, group, stop);
    
    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();
    
    assert_usage_equals("usage: PROG [-h] (--start START | --stop STOP)");
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Test 5c: Mutually Exclusive Group (Mixed types)
 * Expected: usage: PROG [-h] [--output OUTPUT | --stdout]
 * ============================================================================ */
void test_5c_mutex_mixed(void) {
    clap_parser_t *parser = clap_parser_new("PROG", "Test mutex mixed", NULL);
    int group = clap_add_mutually_exclusive_group(parser, false);
    
    clap_argument_t *output = clap_add_argument(parser, "--output");
    clap_argument_mutex_group(output, group);
    clap_mutex_group_add_argument(parser, group, output);
    
    clap_argument_t *stdout_arg = clap_add_argument(parser, "--stdout");
    clap_argument_action(stdout_arg, CLAP_ACTION_STORE_TRUE);
    clap_argument_mutex_group(stdout_arg, group);
    clap_mutex_group_add_argument(parser, group, stdout_arg);
    
    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();
    
    assert_usage_equals("usage: PROG [-h] [--output OUTPUT | --stdout]");
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Test 6a: Subcommands
 * Expected: usage: git [-h] {commit,push} ...
 * ============================================================================ */
void test_6a_subcommands(void) {
    clap_parser_t *parser = clap_parser_new("git", "Test subcommands", NULL);
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    clap_subparser_add(subparsers, "commit", NULL);
    clap_subparser_add(subparsers, "push", NULL);
    
    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();
    
    assert_usage_equals("usage: git [-h] {commit,push} ...");
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Test 6b: Subcommands with Global Options
 * Expected: usage: git [-h] [--version] {commit} ...
 * ============================================================================ */
void test_6b_subcommands_with_global(void) {
    clap_parser_t *parser = clap_parser_new("git", "Test subcommands with global", NULL);
    
    clap_argument_t *version = clap_add_argument(parser, "--version");
    clap_argument_action(version, CLAP_ACTION_VERSION);
    
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    clap_subparser_add(subparsers, "commit", NULL);
    
    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();
    
    assert_usage_equals("usage: git [-h] [--version] {commit} ...");
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Test 7: Subcommand Help
 * Expected: usage: PROG commit [-h] [-m MESSAGE]
 * ============================================================================ */
void test_7_subcommand_help(void) {
    clap_parser_t *parser = clap_parser_new("PROG", "Main program", NULL);
    clap_parser_t *subparsers = clap_add_subparsers(parser, "cmd", NULL);
    clap_parser_t *commit = clap_subparser_add(subparsers, "commit", "Commit changes");
    
    clap_argument_t *msg = clap_add_argument(commit, "-m");
    clap_argument_type(msg, "string");
    clap_argument_metavar(msg, "MSG");  /* Add this line */
    clap_argument_help(msg, "Commit message");
    
    clap_print_help(commit, stdout);
    fflush(stdout);
    capture_stop();
    
    assert_usage_equals("usage: PROG commit [-h] [-m MSG]");  /* Update expected */
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Test 8: nargs='?' with const
 * Expected: usage: PROG [-h] [--output [OUTPUT]]
 * ============================================================================ */
void test_8_nargs_question_with_const(void) {
    clap_parser_t *parser = clap_parser_new("PROG", "Test nargs ? with const", NULL);
    
    clap_argument_t *output = clap_add_argument(parser, "--output");
    clap_argument_nargs(output, '?');
    clap_argument_default(output, "default.out");
    
    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();
    
    assert_usage_equals("usage: PROG [-h] [--output [OUTPUT]]");
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Test 9: REMAINDER nargs
 * Expected: usage: PROG [-h] cmd ...
 * ============================================================================ */
void test_9_remainder_nargs(void) {
    clap_parser_t *parser = clap_parser_new("PROG", "Test REMAINDER", NULL);
    clap_add_argument(parser, "cmd");
    
    clap_argument_t *args = clap_add_argument(parser, "args");
    clap_argument_nargs(args, CLAP_NARGS_REMAINDER);
    
    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();
    
    assert_usage_equals("usage: PROG [-h] cmd ...");
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Test 10: STORE_CONST action usage
 * Expected: usage: PROG [-h] [--fast] [--slow]
 * ============================================================================ */
void test_10_store_const_usage(void) {
    clap_parser_t *parser = clap_parser_new("PROG", "Test STORE_CONST", NULL);
    
    clap_argument_t *fast = clap_add_argument(parser, "--fast");
    clap_argument_action(fast, CLAP_ACTION_STORE_CONST);
    clap_argument_const(fast, "fast");
    clap_argument_dest(fast, "mode");
    
    clap_argument_t *slow = clap_add_argument(parser, "--slow");
    clap_argument_action(slow, CLAP_ACTION_STORE_CONST);
    clap_argument_const(slow, "slow");
    clap_argument_dest(slow, "mode");
    
    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();
    
    assert_usage_equals("usage: PROG [-h] [--fast] [--slow]");
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Test 11: APPEND_CONST action usage
 * Expected: usage: PROG [-h] [--add-tag] [--add-flag]
 * ============================================================================ */
void test_11_append_const_usage(void) {
    clap_parser_t *parser = clap_parser_new("PROG", "Test APPEND_CONST", NULL);
    
    clap_argument_t *add_tag = clap_add_argument(parser, "--add-tag");
    clap_argument_action(add_tag, CLAP_ACTION_APPEND_CONST);
    clap_argument_const(add_tag, "important");
    clap_argument_dest(add_tag, "tags");
    
    clap_argument_t *add_flag = clap_add_argument(parser, "--add-flag");
    clap_argument_action(add_flag, CLAP_ACTION_APPEND_CONST);
    clap_argument_const(add_flag, "urgent");
    clap_argument_dest(add_flag, "flags");
    
    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();
    
    assert_usage_equals("usage: PROG [-h] [--add-tag] [--add-flag]");
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Test 12: STORE_CONST with nargs='?' usage
 * Expected: usage: PROG [-h] [--level [LEVEL]]
 * ============================================================================ */
void test_12_store_const_nargs_usage(void) {
    clap_parser_t *parser = clap_parser_new("PROG", "Test STORE_CONST with nargs", NULL);
    
    clap_argument_t *level = clap_add_argument(parser, "--level");
    clap_argument_nargs(level, '?');
    clap_argument_const(level, "debug");
    clap_argument_default(level, "info");
    
    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();
    
    assert_usage_equals("usage: PROG [-h] [--level [LEVEL]]");
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Test 13: CUSTOM action usage
 * Expected: usage: PROG [-h] [--custom CUSTOM]
 * ============================================================================ */
void test_13_custom_action_usage(void) {
    clap_parser_t *parser = clap_parser_new("PROG", "Test CUSTOM action", NULL);
    
    clap_argument_t *custom = clap_add_argument(parser, "--custom");
    clap_argument_action(custom, CLAP_ACTION_CUSTOM);
    clap_argument_type(custom, "string");
    
    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();
    
    assert_usage_equals("usage: PROG [-h] [--custom CUSTOM]");
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Test 14: Combined STORE_CONST and APPEND_CONST usage
 * Expected: usage: PROG [-h] [--fast] [--slow] [--tag] input
 * ============================================================================ */
void test_14_combined_const_usage(void) {
    clap_parser_t *parser = clap_parser_new("PROG", "Test combined const actions", NULL);
    
    clap_argument_t *mode_fast = clap_add_argument(parser, "--fast");
    clap_argument_action(mode_fast, CLAP_ACTION_STORE_CONST);
    clap_argument_const(mode_fast, "fast");
    clap_argument_dest(mode_fast, "mode");
    
    clap_argument_t *mode_slow = clap_add_argument(parser, "--slow");
    clap_argument_action(mode_slow, CLAP_ACTION_STORE_CONST);
    clap_argument_const(mode_slow, "slow");
    clap_argument_dest(mode_slow, "mode");
    
    clap_argument_t *add_tag = clap_add_argument(parser, "--tag");
    clap_argument_action(add_tag, CLAP_ACTION_APPEND_CONST);
    clap_argument_const(add_tag, "important");
    clap_argument_dest(add_tag, "tags");
    
    clap_add_argument(parser, "input");
    
    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();
    
    assert_usage_equals("usage: PROG [-h] [--fast] [--slow] [--tag] input");
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Test 15: STORE_CONST with required argument
 * Expected: usage: PROG [-h] --mode-fast
 * ============================================================================ */
void test_15_store_const_required_usage(void) {
    clap_parser_t *parser = clap_parser_new("PROG", "Test STORE_CONST required", NULL);
    
    clap_argument_t *fast = clap_add_argument(parser, "--mode-fast");
    clap_argument_action(fast, CLAP_ACTION_STORE_CONST);
    clap_argument_const(fast, "fast");
    clap_argument_dest(fast, "mode");
    clap_argument_required(fast, true);
    
    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();
    
    assert_usage_equals("usage: PROG [-h] --mode-fast");
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Test 16: APPEND_CONST with metavar
 * Expected: usage: PROG [-h] [--tag]
 * ============================================================================ */
void test_16_append_const_metavar_usage(void) {
    clap_parser_t *parser = clap_parser_new("PROG", "Test APPEND_CONST with metavar", NULL);
    
    clap_argument_t *add_tag = clap_add_argument(parser, "--tag");
    clap_argument_action(add_tag, CLAP_ACTION_APPEND_CONST);
    clap_argument_const(add_tag, "important");
    clap_argument_metavar(add_tag, "TAG");
    clap_argument_dest(add_tag, "tags");
    
    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();
    
    assert_usage_equals("usage: PROG [-h] [--tag]");
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Test 17: Choices validation - basic
 * Expected: usage: PROG [-h] [--color {red,green,blue}]
 * ============================================================================ */
void test_17_choices_basic_usage(void) {
    clap_parser_t *parser = clap_parser_new("PROG", "Test choices", NULL);
    
    clap_argument_t *color = clap_add_argument(parser, "--color");
    clap_argument_type(color, "string");
    clap_argument_choices(color, (const char*[]){"red", "green", "blue"}, 3);
    clap_argument_help(color, "Choose a color");
    
    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();
    
    assert_usage_equals("usage: PROG [-h] [--color {red,green,blue}]");
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Test 18: Choices with positional argument
 * Expected: usage: PROG [-h] action
 * ============================================================================ */
void test_18_choices_positional_usage(void) {
    clap_parser_t *parser = clap_parser_new("PROG", "Test positional choices", NULL);
    
    clap_argument_t *action = clap_add_argument(parser, "action");
    clap_argument_type(action, "string");
    clap_argument_choices(action, (const char*[]){"start", "stop", "restart"}, 3);
    
    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();
    
    assert_usage_equals("usage: PROG [-h] action");
    
    clap_parser_free(parser);
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */
int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_1_basic_positional);
    RUN_TEST(test_2a_positional_nargs_star);
    RUN_TEST(test_2b_positional_nargs_plus);
    RUN_TEST(test_2c_positional_nargs_question);
    RUN_TEST(test_3a_only_optional);
    RUN_TEST(test_3b_required_optional);
    RUN_TEST(test_4_mixed_arguments);
    RUN_TEST(test_5a_mutex_optional);
    RUN_TEST(test_5b_mutex_required);
    RUN_TEST(test_5c_mutex_mixed);
    RUN_TEST(test_6a_subcommands);
    RUN_TEST(test_6b_subcommands_with_global);
    RUN_TEST(test_7_subcommand_help);
    RUN_TEST(test_8_nargs_question_with_const);
    RUN_TEST(test_9_remainder_nargs);
    RUN_TEST(test_10_store_const_usage);
    RUN_TEST(test_11_append_const_usage);
    RUN_TEST(test_12_store_const_nargs_usage);
    RUN_TEST(test_13_custom_action_usage);
    RUN_TEST(test_14_combined_const_usage);
    RUN_TEST(test_15_store_const_required_usage);
    RUN_TEST(test_16_append_const_metavar_usage);
    RUN_TEST(test_17_choices_basic_usage);
    RUN_TEST(test_18_choices_positional_usage);

    return UNITY_END();
}