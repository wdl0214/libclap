/**
 * @file clap_types.h
 * @brief Type definitions for libclap
 */

#ifndef CLAP_TYPES_H
#define CLAP_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <clap/clap_error.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
typedef struct clap_parser_s clap_parser_t;
typedef struct clap_argument_s clap_argument_t;
typedef struct clap_namespace_s clap_namespace_t;

/**
 * @brief Type conversion handler function pointer
 */
typedef bool (*clap_type_handler_t)(
    const char *input,
    void *output,
    size_t output_size,
    clap_error_t *error
);

/**
 * @brief Custom action handler function pointer
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
bool clap_type_string_handler(const char *input, void *output, 
                               size_t output_size, clap_error_t *error);
bool clap_type_int_handler(const char *input, void *output,
                            size_t output_size, clap_error_t *error);
bool clap_type_float_handler(const char *input, void *output,
                              size_t output_size, clap_error_t *error);
bool clap_type_bool_handler(const char *input, void *output,
                             size_t output_size, clap_error_t *error);

#ifdef __cplusplus
}
#endif

#endif /* CLAP_TYPES_H */
