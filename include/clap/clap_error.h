/**
 * @file clap_error.h
 * @brief Error handling for libclap
 */

#ifndef CLAP_ERROR_H
#define CLAP_ERROR_H

#include <stdbool.h>
#include <stdarg.h>
#include <clap/clap_export.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Error codes */
#define CLAP_ERR_NONE                0
#define CLAP_ERR_INVALID_ARGUMENT    1
#define CLAP_ERR_MISSING_VALUE       2
#define CLAP_ERR_TYPE_CONVERSION     3
#define CLAP_ERR_INVALID_CHOICE      4
#define CLAP_ERR_MUTUALLY_EXCLUSIVE  5
#define CLAP_ERR_REQUIRED_MISSING    6
#define CLAP_ERR_UNRECOGNIZED        7
#define CLAP_ERR_TOO_MANY_ARGS       8
#define CLAP_ERR_TOO_FEW_ARGS        9
#define CLAP_ERR_MEMORY             10
#define CLAP_ERR_SUBCOMMAND_FAILED  11
#define CLAP_ERR_DEPENDENCY_VIOLATION 12
#define CLAP_ERR_CUSTOM             100

/**
 * @brief Error information structure
 */
typedef struct clap_error_s {
    int code;                       /**< Error code */
    char message[512];              /**< Error message */
    const char *argument_name;      /**< Related argument name */
    const char *invalid_value;      /**< Invalid value that caused error */
    const char *subcommand_name;    /**< Subcommand name if applicable */
} clap_error_t;

/**
 * @brief Initialize an error structure to NONE / empty.
 * @param error Error struct to initialize (must not be NULL).
 */
CLAP_EXPORT void clap_error_init(clap_error_t *error);

/**
 * @brief Set error code and printf-style message.
 * @param error  Error struct to fill.
 * @param code   Error code (CLAP_ERR_* constant).
 * @param format printf-style format string.
 * @param ...    printf arguments.
 */
CLAP_EXPORT void clap_error_set(clap_error_t *error, int code, const char *format, ...);

/**
 * @brief Set error code and message with va_list.
 * @param error  Error struct to fill.
 * @param code   Error code (CLAP_ERR_* constant).
 * @param format printf-style format string.
 * @param ap     va_list of format arguments.
 */
CLAP_EXPORT void clap_error_vset(clap_error_t *error, int code, const char *format, va_list ap);

/**
 * @brief Get a human-readable description for an error code.
 * @param code Error code (CLAP_ERR_* constant).
 * @return A static NUL-terminated string.  Never returns NULL.
 */
CLAP_EXPORT const char* clap_strerror(int code);

#ifdef __cplusplus
}
#endif

#endif /* CLAP_ERROR_H */
