/**
 * @file custom_type.c
 * @brief Demonstrate custom type validation with clap_register_type()
 *
 * == Part 1: Validation-only custom types ==
 *
 * Custom type handlers validate input at parse time.  The raw string is
 * stored in the namespace — the handler's converted output is discarded.
 * This is useful when you want to reject bad input early but don't need
 * a special binary value type.
 *
 * == Part 2: Custom type + CLAP_ACTION_CUSTOM (typed storage) ==
 *
 * When you need to persist a typed value (int, float, struct, ...),
 * combine clap_register_type() with CLAP_ACTION_CUSTOM:
 *   1. clap_register_type()     — validate + produce typed value
 *   2. CLAP_ACTION_CUSTOM        — store the typed value in the namespace
 *   3. clap_argument_handler()   — your custom action handler
 *      calls the type handler and stores the result via clap_namespace_set_*()
 *
 * Usage:
 *   ./custom_type --port 8080                              # Part 1
 *   ./custom_type --port 99999                             # Error: port out of range
 *   ./custom_type --port abc                               # Error: not a number
 *   ./custom_type --port 80 --timeout 5000                 # Part 1 + Part 2
 *   ./custom_type --port 80 --timeout -1                   # Error: timeout negative
 */

#include <clap/clap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/* ============================================================================
 * Custom type handler: "port"
 *
 * Validates that the input is an integer in [1, 65535].  The converted
 * output (int) is written to the handler buffer but is NOT persisted in
 * the namespace.  The raw string is stored instead.
 * ============================================================================ */

static bool port_type_handler(const char *input,
                               void *output,
                               size_t output_size,
                               clap_error_t *error) {
    /* Validate output_size matches what was passed to clap_register_type */
    if (output_size != sizeof(int)) {
        clap_error_set(error, CLAP_ERR_TYPE_CONVERSION,
                       "Internal error: unexpected output size");
        return false;
    }

    /* Convert string to integer */
    errno = 0;
    char *endptr = NULL;
    long val = strtol(input, &endptr, 10);

    if (errno != 0 || endptr == input || *endptr != '\0') {
        clap_error_set(error, CLAP_ERR_TYPE_CONVERSION,
                       "Invalid port number: '%s'", input);
        return false;
    }

    if (val < 1 || val > 65535) {
        clap_error_set(error, CLAP_ERR_TYPE_CONVERSION,
                       "Port %ld out of range [1, 65535]", val);
        return false;
    }

    /* Write the converted value — not persisted by the library, but
     * required contractually for the type_handler_t signature. */
    *(int *)output = (int)val;
    return true;
}

/* ============================================================================
 * Custom type handler: "hex_color"
 *
 * Validates that the input is a 6-character hex color like "FF00CC".
 * ============================================================================ */

static bool hex_color_handler(const char *input,
                               void *output,
                               size_t output_size,
                               clap_error_t *error) {
    (void)output;
    (void)output_size;

    size_t len = strlen(input);
    if (len != 6) {
        clap_error_set(error, CLAP_ERR_TYPE_CONVERSION,
                       "Hex color must be exactly 6 hex digits, got '%s'", input);
        return false;
    }

    for (size_t i = 0; i < 6; i++) {
        if (!((input[i] >= '0' && input[i] <= '9') ||
              (input[i] >= 'A' && input[i] <= 'F') ||
              (input[i] >= 'a' && input[i] <= 'f'))) {
            clap_error_set(error, CLAP_ERR_TYPE_CONVERSION,
                           "Invalid hex digit '%c' in color '%s'", input[i], input);
            return false;
        }
    }

    return true;
}

/* ============================================================================
 * Custom type handler: "nonempty"
 *
 * Simple validator — rejects empty strings.  Demonstrates a handler that
 * does not write to the output buffer (pure validation).
 * ============================================================================ */

static bool nonempty_handler(const char *input,
                              void *output,
                              size_t output_size,
                              clap_error_t *error) {
    (void)output;
    (void)output_size;

    if (!input || input[0] == '\0') {
        clap_error_set(error, CLAP_ERR_TYPE_CONVERSION,
                       "Value cannot be empty");
        return false;
    }
    return true;
}

/* ============================================================================
 * Custom type handler: "duration_ms"
 *
 * Validates a non-negative integer (milliseconds).  The converted int
 * value IS used — see timeout_action_handler below.
 * ============================================================================ */

static bool duration_ms_type_handler(const char *input,
                                      void *output,
                                      size_t output_size,
                                      clap_error_t *error) {
    if (output_size != sizeof(int)) {
        clap_error_set(error, CLAP_ERR_TYPE_CONVERSION,
                       "Internal error: unexpected output size");
        return false;
    }

    errno = 0;
    char *endptr = NULL;
    long val = strtol(input, &endptr, 10);

    if (errno != 0 || endptr == input || *endptr != '\0') {
        clap_error_set(error, CLAP_ERR_TYPE_CONVERSION,
                       "Invalid number: '%s'", input);
        return false;
    }

    if (val < 0) {
        clap_error_set(error, CLAP_ERR_TYPE_CONVERSION,
                       "Timeout cannot be negative: %ld", val);
        return false;
    }

    *(int *)output = (int)val;
    return true;
}

/* ============================================================================
 * CUSTOM action handler for --timeout
 *
 * Combines with duration_ms_type_handler to store a typed int value:
 *   1. Call the type handler to validate + convert to int
 *   2. Store the int in the namespace via clap_namespace_set_int()
 *
 * This is the key pattern: type handler validates, custom action persists.
 * ============================================================================ */

static bool timeout_action_handler(clap_parser_t *parser,
                                    clap_argument_t *arg,
                                    clap_namespace_t *ns,
                                    const char **values,
                                    size_t value_count,
                                    void *user_data,
                                    clap_error_t *error) {
    (void)parser;
    (void)arg;
    (void)user_data;

    if (value_count == 0) return true;

    /* Reuse the registered type handler for validation + conversion */
    int timeout_ms;
    if (!duration_ms_type_handler(values[0], &timeout_ms,
                                   sizeof(timeout_ms), error)) {
        return false;
    }

    /* Store as a proper int — retrievable via clap_namespace_get_int() */
    return clap_namespace_set_int(ns, "timeout", timeout_ms);
}

/* ============================================================================
 * Main
 * ============================================================================ */

int main(int argc, char *argv[]) {
    clap_parser_t *parser = clap_parser_new("custom_type",
        "Demonstrate custom type validation", NULL);

    /* Register custom types before adding arguments that use them.
     * The output_size parameter must match what the handler expects
     * for its output buffer. */
    clap_register_type(parser, "port",       port_type_handler,   sizeof(int));
    clap_register_type(parser, "hex_color",  hex_color_handler,   0);
    clap_register_type(parser, "nonempty",   nonempty_handler,    0);

    /* Arguments using custom types */
    clap_argument_t *port_arg = clap_add_argument(parser, "--port/-p");
    clap_argument_type(port_arg, "port");
    clap_argument_help(port_arg, "Port number (1-65535)");
    clap_argument_required(port_arg, true);

    clap_argument_t *color_arg = clap_add_argument(parser, "--color/-c");
    clap_argument_type(color_arg, "hex_color");
    clap_argument_help(color_arg, "Hex color code (e.g. FF00CC)");

    clap_argument_t *name_arg = clap_add_argument(parser, "--name/-n");
    clap_argument_type(name_arg, "nonempty");
    clap_argument_help(name_arg, "Non-empty name string");

    /* ========================================================================
     * Part 2: Custom type + CLAP_ACTION_CUSTOM for typed storage
     *
     * Unlike Part 1 where values are stored as strings, this argument
     * stores a proper int in the namespace.
     * ======================================================================== */

    /* Register the type handler for validation + conversion */
    clap_register_type(parser, "duration_ms", duration_ms_type_handler, sizeof(int));

    clap_argument_t *timeout = clap_add_argument(parser, "--timeout/-t");
    clap_argument_type(timeout, "duration_ms");   /* validation via type handler */
    clap_argument_action(timeout, CLAP_ACTION_CUSTOM);  /* custom storage */
    clap_argument_handler(timeout, timeout_action_handler);
    clap_argument_help(timeout, "Timeout in milliseconds");
    clap_argument_metavar(timeout, "MS");

    /* Parse */
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};

    clap_parse_result_t result = clap_parse_args(parser, argc, argv, &ns, &error);

    if (result == CLAP_PARSE_ERROR) {
        clap_print_help_on_error(parser, &error, stderr);
        clap_parser_free(parser);
        return 1;
    }

    if (result == CLAP_PARSE_HELP || result == CLAP_PARSE_VERSION) {
        clap_parser_free(parser);
        clap_namespace_free(ns);
        return 0;
    }

    /* Retrieve values — custom types are always stored as strings */
    const char *port = NULL;
    if (clap_namespace_get_string(ns, "port", &port)) {
        printf("port: %s\n", port);
        /* Convert to int yourself if needed */
        int port_int = atoi(port);
        printf("      (as int: %d)\n", port_int);
    }

    const char *color = NULL;
    if (clap_namespace_get_string(ns, "color", &color)) {
        printf("color: %s\n", color);
    }

    const char *name = NULL;
    if (clap_namespace_get_string(ns, "name", &name)) {
        printf("name:  %s\n", name);
    }

    /* Retrieve typed value — CLAP_ACTION_CUSTOM stored it as int */
    int timeout_ms = 0;
    if (clap_namespace_get_int(ns, "timeout", &timeout_ms)) {
        printf("timeout: %d ms\n", timeout_ms);
    }

    clap_namespace_free(ns);
    clap_parser_free(parser);
    return 0;
}
