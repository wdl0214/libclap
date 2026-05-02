/**
 * @file clap_types.h
 * @brief Type definitions for libclap
 */

#ifndef CLAP_TYPES_H
#define CLAP_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <clap/clap_error.h>
#include <clap/clap_export.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct clap_parser_s
 * @brief Parsing engine state and configuration.
 *
 * Opaque structure.  Created with clap_parser_new(), configured via
 * clap_parser_set_*() and clap_add_argument() family, then passed
 * to clap_parse_args().  Members are internal.
 */
typedef struct clap_parser_s clap_parser_t;

/**
 * @struct clap_argument_s
 * @brief Argument definition (positional or optional).
 *
 * Opaque structure.  Created by clap_add_argument(), configured via
 * clap_argument_*() setter functions.  Not freed directly — owned
 * by the parser.
 */
typedef struct clap_argument_s clap_argument_t;

/**
 * @struct clap_namespace_s
 * @brief Container for parsed argument values.
 *
 * Opaque structure.  Allocated by clap_parse_args(), freed with
 * clap_namespace_free().  Values are retrieved via the
 * clap_namespace_get_*() accessor family.
 */
typedef struct clap_namespace_s clap_namespace_t;

/**
 * @brief Parse result codes returned by clap_parse_args()
 *
 * CLAP_PARSE_SUCCESS (0):  Parsing completed successfully.
 * CLAP_PARSE_ERROR  (1):  An error occurred; check clap_error_t.
 * CLAP_PARSE_HELP   (2):  --help or -h was requested; help has been
 *                         printed to stdout.  The caller should exit
 *                         with code 0.
 * CLAP_PARSE_VERSION(3):  --version was requested; version has been
 *                         printed to stdout.  The caller should exit
 *                         with code 0.
 */
typedef enum {
    CLAP_PARSE_SUCCESS = 0,
    CLAP_PARSE_ERROR   = 1,
    CLAP_PARSE_HELP    = 2,
    CLAP_PARSE_VERSION = 3,
} clap_parse_result_t;

/**
 * @brief Type conversion handler function pointer
 *
 * Converts a string input into a typed value.  Built-in handlers exist
 * for "string", "int", "float", and "bool".  Register custom handlers
 * with clap_register_type().
 *
 * @param input    NUL-terminated string to convert.
 * @param output   Pointer to output buffer of @p output_size bytes.
 *                 The handler writes the converted value here.
 * @param output_size  Size of the @p output buffer in bytes.
 * @param error    Optional (may be NULL).  Set on failure with a
 *                 descriptive message.
 * @return true on success, false on conversion failure (error is set).
 */
typedef bool (*clap_type_handler_t)(
    const char *input,
    void *output,
    size_t output_size,
    clap_error_t *error
);

/**
 * @brief Custom action handler function pointer
 *
 * Invoked when a CUSTOM-action argument is encountered during parsing.
 * The handler receives the full parser context and the list of values
 * associated with this argument occurrence.
 *
 * @param parser     The parser being used for this parse run.
 *                   The handler may inspect parser state but should not
 *                   add/modify arguments or groups during parsing.
 * @param argument   The argument definition that triggered this handler.
 * @param namespace  The result namespace.  Write parsed values here
 *                   via clap_namespace_set_* / clap_namespace_append_*.
 * @param values     Array of raw string values for this occurrence.
 *                   For nargs=1 or the first value of nargs>1, this
 *                   contains one element.  The handler is called once
 *                   per argument occurrence.
 * @param value_count  Number of elements in @p values.
 * @param user_data  Opaque pointer set via
 *                   clap_argument_handler(arg, handler)->action_data.
 *                   May be NULL.
 * @param error      Set on failure with a descriptive message.
 * @return true on success, false on failure (parse aborts with error).
 *
 * @par Example
 * @code
 * static bool count_lines(clap_parser_t *p, clap_argument_t *a,
 *                         clap_namespace_t *ns, const char **vals,
 *                         size_t n, void *data, clap_error_t *err) {
 *     (void)p; (void)a; (void)data;
 *     FILE *f = fopen(vals[0], "r");
 *     if (!f) { clap_error_set(err, CLAP_ERR_CUSTOM, "cannot open"); return false; }
 *     long lines = 0; int c;
 *     while ((c = fgetc(f)) != EOF) if (c == '\n') lines++;
 *     rewind(f);
 *     while ((c = fgetc(f)) != EOF) if (c == '\n') lines++;
 *     fclose(f);
 *     return clap_namespace_set_int(ns, clap_buffer_cstr(a->dest), (int)lines);
 * }
 * @endcode
 */
typedef bool (*clap_action_handler_t)(
    clap_parser_t *parser,
    clap_argument_t *argument,
    clap_namespace_t *namespace,
    const char **values,
    size_t value_count,
    void *user_data,
    clap_error_t *error
);

/* Built-in type handlers */
CLAP_EXPORT bool clap_type_string_handler(const char *input, void *output,
                               size_t output_size, clap_error_t *error);
CLAP_EXPORT bool clap_type_int_handler(const char *input, void *output,
                            size_t output_size, clap_error_t *error);
CLAP_EXPORT bool clap_type_float_handler(const char *input, void *output,
                              size_t output_size, clap_error_t *error);
CLAP_EXPORT bool clap_type_bool_handler(const char *input, void *output,
                             size_t output_size, clap_error_t *error);

#ifdef __cplusplus
}
#endif

#endif /* CLAP_TYPES_H */
