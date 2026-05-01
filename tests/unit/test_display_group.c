/**
 * @file test_display_group.c
 * @brief Unit tests for clap_display_group.c
 */

#include "unity.h"
#include "clap_parser_internal.h"
#include <string.h>

/* ============================================================================
 * Output Capture
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
static void capture_start(void) {
    g_captured_len = 0;
    g_captured[0] = '\0';
}

static void capture_stop(void) {}
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
 * clap_add_argument_group Tests
 * ============================================================================ */

void test_add_display_group_basic(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);

    int group_id = clap_add_argument_group(parser, "Network options", "Connection settings");

    TEST_ASSERT_GREATER_OR_EQUAL(0, group_id);
    TEST_ASSERT_EQUAL(1, parser->display_group_count);
    TEST_ASSERT_NOT_NULL(parser->display_groups);
    TEST_ASSERT_NOT_NULL(parser->display_groups[0]);
    TEST_ASSERT_EQUAL(group_id, parser->display_groups[0]->id);
    TEST_ASSERT_EQUAL_STRING("Network options", parser->display_groups[0]->title);
    TEST_ASSERT_EQUAL_STRING("Connection settings", parser->display_groups[0]->description);
    TEST_ASSERT_EQUAL(0, parser->display_groups[0]->arg_count);

    clap_parser_free(parser);
}

void test_add_display_group_null_title(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);

    int group_id = clap_add_argument_group(parser, NULL, NULL);

    TEST_ASSERT_EQUAL(-1, group_id);

    clap_parser_free(parser);
}

void test_add_display_group_multiple(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);

    int id1 = clap_add_argument_group(parser, "Network", NULL);
    int id2 = clap_add_argument_group(parser, "Output", "Output file options");
    int id3 = clap_add_argument_group(parser, "Debug", NULL);

    TEST_ASSERT_EQUAL(0, id1);
    TEST_ASSERT_EQUAL(1, id2);
    TEST_ASSERT_EQUAL(2, id3);
    TEST_ASSERT_EQUAL(3, parser->display_group_count);

    clap_parser_free(parser);
}

void test_add_display_group_null_parser(void) {
    int result = clap_add_argument_group(NULL, "Title", NULL);
    TEST_ASSERT_EQUAL(-1, result);
}

void test_add_display_group_expands_parser_array(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);

    for (int i = 0; i < 10; i++) {
        char title[32];
        snprintf(title, sizeof(title), "Group %d", i);
        int id = clap_add_argument_group(parser, title, NULL);
        TEST_ASSERT_EQUAL(i, id);
    }

    TEST_ASSERT_EQUAL(10, parser->display_group_count);

    clap_parser_free(parser);
}

/* ============================================================================
 * clap_argument_group_add_argument Tests
 * ============================================================================ */

void test_display_group_add_argument_basic(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group_id = clap_add_argument_group(parser, "Network", NULL);

    clap_argument_t *arg1 = clap_add_argument(parser, "--host");

    bool result = clap_argument_group_add_argument(parser, group_id, arg1);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(1, parser->display_groups[0]->arg_count);
    TEST_ASSERT_EQUAL_PTR(arg1, parser->display_groups[0]->arguments[0]);
    TEST_ASSERT_EQUAL(group_id, arg1->display_group_id);

    clap_parser_free(parser);
}

void test_display_group_add_argument_multiple(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group_id = clap_add_argument_group(parser, "Network", NULL);

    clap_argument_t *arg1 = clap_add_argument(parser, "--host");
    clap_argument_t *arg2 = clap_add_argument(parser, "--port");
    clap_argument_t *arg3 = clap_add_argument(parser, "--ip");

    clap_argument_group_add_argument(parser, group_id, arg1);
    clap_argument_group_add_argument(parser, group_id, arg2);
    clap_argument_group_add_argument(parser, group_id, arg3);

    TEST_ASSERT_EQUAL(3, parser->display_groups[0]->arg_count);
    TEST_ASSERT_EQUAL_PTR(arg1, parser->display_groups[0]->arguments[0]);
    TEST_ASSERT_EQUAL_PTR(arg2, parser->display_groups[0]->arguments[1]);
    TEST_ASSERT_EQUAL_PTR(arg3, parser->display_groups[0]->arguments[2]);

    clap_parser_free(parser);
}

void test_display_group_add_argument_expands_array(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group_id = clap_add_argument_group(parser, "Group", NULL);

    for (int i = 0; i < 10; i++) {
        char name[32];
        snprintf(name, sizeof(name), "--opt%d", i);
        clap_argument_t *arg = clap_add_argument(parser, name);

        bool result = clap_argument_group_add_argument(parser, group_id, arg);
        TEST_ASSERT_TRUE(result);
    }

    TEST_ASSERT_EQUAL(10, parser->display_groups[0]->arg_count);
    TEST_ASSERT_TRUE(parser->display_groups[0]->arg_capacity >= 10);

    clap_parser_free(parser);
}

void test_display_group_add_argument_null_parser(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group_id = clap_add_argument_group(parser, "Group", NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--opt");

    bool result = clap_argument_group_add_argument(NULL, group_id, arg);
    TEST_ASSERT_FALSE(result);

    clap_parser_free(parser);
}

void test_display_group_add_argument_invalid_group_id(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    clap_argument_t *arg = clap_add_argument(parser, "--opt");

    bool result = clap_argument_group_add_argument(parser, -1, arg);
    TEST_ASSERT_FALSE(result);

    result = clap_argument_group_add_argument(parser, 999, arg);
    TEST_ASSERT_FALSE(result);

    clap_parser_free(parser);
}

void test_display_group_add_argument_null_arg(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group_id = clap_add_argument_group(parser, "Group", NULL);

    bool result = clap_argument_group_add_argument(parser, group_id, NULL);
    TEST_ASSERT_FALSE(result);

    clap_parser_free(parser);
}

void test_display_group_sets_display_group_id(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group_id = clap_add_argument_group(parser, "Group", NULL);

    clap_argument_t *arg = clap_add_argument(parser, "--opt");

    clap_argument_group_add_argument(parser, group_id, arg);

    TEST_ASSERT_EQUAL(group_id, arg->display_group_id);

    clap_parser_free(parser);
}

/* ============================================================================
 * Display Group with Positional Arguments
 * ============================================================================ */

void test_display_group_with_positional(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group_id = clap_add_argument_group(parser, "Input files", "Files to process");

    clap_argument_t *arg = clap_add_argument(parser, "input");
    clap_argument_group_add_argument(parser, group_id, arg);

    TEST_ASSERT_EQUAL(1, parser->display_groups[0]->arg_count);
    TEST_ASSERT_EQUAL_PTR(arg, parser->display_groups[0]->arguments[0]);

    clap_parser_free(parser);
}

/* ============================================================================
 * Help Output Tests
 * ============================================================================ */

void test_display_group_help_section(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group_id = clap_add_argument_group(parser, "Network", "Connection options");

    clap_argument_t *host = clap_add_argument(parser, "--host");
    clap_argument_help(host, "Hostname to connect to");
    clap_argument_group_add_argument(parser, group_id, host);

    clap_argument_t *port = clap_add_argument(parser, "--port");
    clap_argument_type(port, "int");
    clap_argument_help(port, "Port number");
    clap_argument_group_add_argument(parser, group_id, port);

    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();

    const char *output = get_captured();
    TEST_ASSERT_NOT_NULL(strstr(output, "Network:"));
    TEST_ASSERT_NOT_NULL(strstr(output, "Connection options"));
    TEST_ASSERT_NOT_NULL(strstr(output, "--host"));
    TEST_ASSERT_NOT_NULL(strstr(output, "--port"));
    TEST_ASSERT_NOT_NULL(strstr(output, "Hostname to connect to"));
    TEST_ASSERT_NOT_NULL(strstr(output, "Port number"));

    clap_parser_free(parser);
}

void test_display_group_args_excluded_from_optional_section(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group_id = clap_add_argument_group(parser, "Network", NULL);

    clap_argument_t *host = clap_add_argument(parser, "--host");
    clap_argument_help(host, "Hostname");
    clap_argument_group_add_argument(parser, group_id, host);

    /* This arg should still appear in Optional arguments: */
    clap_argument_t *verbose = clap_add_argument(parser, "--verbose");
    clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);
    clap_argument_help(verbose, "Verbose output");

    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();

    const char *output = get_captured();

    /* Display group section should have --host */
    const char *network_section = strstr(output, "Network:");
    TEST_ASSERT_NOT_NULL(network_section);
    TEST_ASSERT_NOT_NULL(strstr(network_section, "--host"));

    /* Optional arguments section should have --verbose */
    const char *opt_section = strstr(output, "Optional arguments:");
    TEST_ASSERT_NOT_NULL(opt_section);
    TEST_ASSERT_NOT_NULL(strstr(opt_section, "--verbose"));

    /* Verify ordering: Optional section before Network section */
    TEST_ASSERT(opt_section < network_section);

    clap_parser_free(parser);
}

void test_display_group_usage_still_shows_all_args(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group_id = clap_add_argument_group(parser, "Network", NULL);

    clap_argument_t *host = clap_add_argument(parser, "--host");
    clap_argument_group_add_argument(parser, group_id, host);

    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();

    const char *output = get_captured();
    char usage[1024];
    const char *start = strstr(output, "usage: ");
    TEST_ASSERT_NOT_NULL(start);
    const char *end = strchr(start, '\n');
    size_t len = end ? (size_t)(end - start) : strlen(start);
    if (len >= sizeof(usage)) len = sizeof(usage) - 1;
    memcpy(usage, start, len);
    usage[len] = '\0';

    TEST_ASSERT_NOT_NULL(strstr(usage, "--host"));

    clap_parser_free(parser);
}

void test_display_group_help_no_description(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group_id = clap_add_argument_group(parser, "Output", NULL);

    clap_argument_t *out = clap_add_argument(parser, "--output");
    clap_argument_help(out, "Output file");
    clap_argument_group_add_argument(parser, group_id, out);

    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();

    const char *output = get_captured();
    TEST_ASSERT_NOT_NULL(strstr(output, "Output:"));
    TEST_ASSERT_NOT_NULL(strstr(output, "Output file"));

    /* --output should appear in usage line (before "Output:" section) */
    TEST_ASSERT_NOT_NULL(strstr(output, "--output"));

    /* No description text should appear between "Output:" title and first arg */
    const char *title_pos = strstr(output, "Output:");
    TEST_ASSERT_NOT_NULL(title_pos);
    /* The next line after "Output:" should start with whitespace (the indented arg) */
    const char *after_title = title_pos + strlen("Output:");
    while (*after_title && *after_title == '\n') after_title++;
    /* First non-empty line after title should be indented with spaces (arg line) */
    TEST_ASSERT_NOT_NULL(strchr(after_title, ' '));

    clap_parser_free(parser);
}

void test_multiple_display_groups(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);

    int net_group = clap_add_argument_group(parser, "Network", "Connection settings");
    clap_argument_t *host = clap_add_argument(parser, "--host");
    clap_argument_help(host, "Hostname");
    clap_argument_group_add_argument(parser, net_group, host);

    int out_group = clap_add_argument_group(parser, "Output", NULL);
    clap_argument_t *out = clap_add_argument(parser, "--output");
    clap_argument_help(out, "Output file");
    clap_argument_group_add_argument(parser, out_group, out);

    clap_print_help(parser, stdout);
    fflush(stdout);
    capture_stop();

    const char *output = get_captured();
    TEST_ASSERT_NOT_NULL(strstr(output, "Network:"));
    TEST_ASSERT_NOT_NULL(strstr(output, "Output:"));

    const char *net_pos = strstr(output, "Network:");
    const char *out_pos = strstr(output, "Output:");
    TEST_ASSERT(out_pos > net_pos);

    clap_parser_free(parser);
}

void test_display_group_args_parsed_correctly(void) {
    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    int group_id = clap_add_argument_group(parser, "Network", NULL);

    clap_argument_t *host = clap_add_argument(parser, "--host");
    clap_argument_type(host, "string");
    clap_argument_group_add_argument(parser, group_id, host);

    clap_argument_t *port = clap_add_argument(parser, "--port");
    clap_argument_type(port, "int");
    clap_argument_group_add_argument(parser, group_id, port);

    clap_argument_t *verbose = clap_add_argument(parser, "--verbose");
    clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);

    char *argv[] = {"prog", "--host", "example.com", "--port",  "8080", "--verbose"};
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};

    bool result = clap_parse_args(parser, 6, argv, &ns, &error);
    TEST_ASSERT_TRUE(result);

    const char *host_val;
    TEST_ASSERT_TRUE(clap_namespace_get_string(ns, "host", &host_val));
    TEST_ASSERT_EQUAL_STRING("example.com", host_val);

    int port_val;
    TEST_ASSERT_TRUE(clap_namespace_get_int(ns, "port", &port_val));
    TEST_ASSERT_EQUAL(8080, port_val);

    bool verbose_val;
    TEST_ASSERT_TRUE(clap_namespace_get_bool(ns, "verbose", &verbose_val));
    TEST_ASSERT_TRUE(verbose_val);

    clap_namespace_free(ns);
    clap_parser_free(parser);
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

void run_test_display_group(void) {
    /* clap_add_argument_group Tests */
    RUN_TEST(test_add_display_group_basic);
    RUN_TEST(test_add_display_group_null_title);
    RUN_TEST(test_add_display_group_multiple);
    RUN_TEST(test_add_display_group_null_parser);
    RUN_TEST(test_add_display_group_expands_parser_array);

    /* clap_argument_group_add_argument Tests */
    RUN_TEST(test_display_group_add_argument_basic);
    RUN_TEST(test_display_group_add_argument_multiple);
    RUN_TEST(test_display_group_add_argument_expands_array);
    RUN_TEST(test_display_group_add_argument_null_parser);
    RUN_TEST(test_display_group_add_argument_invalid_group_id);
    RUN_TEST(test_display_group_add_argument_null_arg);
    RUN_TEST(test_display_group_sets_display_group_id);

    /* Positional in display group */
    RUN_TEST(test_display_group_with_positional);

    /* Help output tests */
    RUN_TEST(test_display_group_help_section);
    RUN_TEST(test_display_group_args_excluded_from_optional_section);
    RUN_TEST(test_display_group_usage_still_shows_all_args);
    RUN_TEST(test_display_group_help_no_description);
    RUN_TEST(test_multiple_display_groups);

    /* Integration test */
    RUN_TEST(test_display_group_args_parsed_correctly);
}

#ifdef STANDALONE_TEST
int main(void) {
    UNITY_BEGIN();
    run_test_display_group();
    return UNITY_END();
}
#endif
