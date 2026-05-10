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

int clap_error_code(const clap_error_t *error) {
    return error ? error->code : CLAP_ERR_NONE;
}

const char* clap_error_message(const clap_error_t *error) {
    return error ? error->message : "";
}

const char* clap_strerror(int code) {
    switch (code) {
        case CLAP_ERR_NONE:
            return CLAP_TR("Success");
        case CLAP_ERR_INVALID_ARGUMENT:
            return CLAP_TR("Invalid argument");
        case CLAP_ERR_MISSING_VALUE:
            return CLAP_TR("Missing required value");
        case CLAP_ERR_TYPE_CONVERSION:
            return CLAP_TR("Type conversion failed");
        case CLAP_ERR_INVALID_CHOICE:
            return CLAP_TR("Invalid choice");
        case CLAP_ERR_MUTUALLY_EXCLUSIVE:
            return CLAP_TR("Mutually exclusive arguments specified");
        case CLAP_ERR_REQUIRED_MISSING:
            return CLAP_TR("Required argument missing");
        case CLAP_ERR_UNRECOGNIZED:
            return CLAP_TR("Unrecognized argument");
        case CLAP_ERR_TOO_MANY_ARGS:
            return CLAP_TR("Too many arguments");
        case CLAP_ERR_TOO_FEW_ARGS:
            return CLAP_TR("Too few arguments");
        case CLAP_ERR_MEMORY:
            return CLAP_TR("Memory allocation failed");
        case CLAP_ERR_SUBCOMMAND_FAILED:
            return CLAP_TR("Subcommand failed");
        case CLAP_ERR_DEPENDENCY_VIOLATION:
            return CLAP_TR("Dependency violation");
        default:
            return CLAP_TR("Unknown error");
    }
}
