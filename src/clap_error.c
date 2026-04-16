/**
* @file clap_error.c
 * @brief Error handling implementation
 */

#include "clap_parser_internal.h"
#include <stdarg.h>
#include <stdio.h>

void clap_error_init(clap_error_t *error) {
    if (error) {
        memset(error, 0, sizeof(*error));
        error->code = CLAP_ERR_NONE;
    }
}

void clap_error_set(clap_error_t *error, int code, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    clap_error_vset(error, code, format, ap);
    va_end(ap);
}

void clap_error_vset(clap_error_t *error, int code, const char *format, va_list ap) {
    if (!error) return;

    error->code = code;

    if (format) {
        vsnprintf(error->message, sizeof(error->message), format, ap);
    } else {
        const char *default_msg = clap_strerror(code);
        strncpy(error->message, default_msg, sizeof(error->message) - 1);
        error->message[sizeof(error->message) - 1] = '\0';
    }
}

const char* clap_strerror(int code) {
    switch (code) {
        case CLAP_ERR_NONE:
            return "Success";
        case CLAP_ERR_INVALID_ARGUMENT:
            return "Invalid argument";
        case CLAP_ERR_MISSING_VALUE:
            return "Missing required value";
        case CLAP_ERR_TYPE_CONVERSION:
            return "Type conversion failed";
        case CLAP_ERR_INVALID_CHOICE:
            return "Invalid choice";
        case CLAP_ERR_MUTUALLY_EXCLUSIVE:
            return "Mutually exclusive arguments specified";
        case CLAP_ERR_REQUIRED_MISSING:
            return "Required argument missing";
        case CLAP_ERR_UNRECOGNIZED:
            return "Unrecognized argument";
        case CLAP_ERR_TOO_MANY_ARGS:
            return "Too many arguments";
        case CLAP_ERR_TOO_FEW_ARGS:
            return "Too few arguments";
        case CLAP_ERR_MEMORY:
            return "Memory allocation failed";
        case CLAP_ERR_SUBCOMMAND_FAILED:
            return "Subcommand failed";
        case CLAP_ERR_DEPENDENCY_VIOLATION:
            return "Dependency violation";
        default:
            return "Unknown error";
    }
}