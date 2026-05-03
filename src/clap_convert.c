/**
 * @file clap_convert.c
 * @brief Type conversion implementation
 */

#include "clap_parser_internal.h"
#include <math.h>

bool clap_type_string_handler(const char *input, void *output,
                               size_t output_size, clap_error_t *error) {
    if (output_size != sizeof(char**)) {
        clap_error_set(error, CLAP_ERR_TYPE_CONVERSION, "Invalid output size");
        return false;
    }
    *(const char**)output = input;
    return true;
}

bool clap_type_int_handler(const char *input, void *output,
                            size_t output_size, clap_error_t *error) {
    if (output_size != sizeof(int)) {
        clap_error_set(error, CLAP_ERR_TYPE_CONVERSION, "Invalid output size");
        return false;
    }

    errno = 0;
    char *endptr;
    long val = strtol(input, &endptr, 0);

    if (errno == ERANGE || val > INT_MAX || val < INT_MIN) {
        clap_error_set(error, CLAP_ERR_TYPE_CONVERSION, "Integer out of range: '%s'", input);
        return false;
    }

    if (endptr == input || *endptr != '\0') {
        clap_error_set(error, CLAP_ERR_TYPE_CONVERSION, "Invalid integer: '%s'", input);
        return false;
    }

    *(int*)output = (int)val;
    return true;
}

bool clap_type_float_handler(const char *input, void *output,
                              size_t output_size, clap_error_t *error) {
    if (output_size != sizeof(double)) {
        clap_error_set(error, CLAP_ERR_TYPE_CONVERSION, "Invalid output size");
        return false;
    }

    errno = 0;
    char *endptr;
    double val = strtod(input, &endptr);

    if (errno == ERANGE) {
        clap_error_set(error, CLAP_ERR_TYPE_CONVERSION, "Float out of range: '%s'", input);
        return false;
    }

    if (endptr == input || *endptr != '\0') {
        clap_error_set(error, CLAP_ERR_TYPE_CONVERSION, "Invalid float: '%s'", input);
        return false;
    }

    if (!clap_isfinite(val)) {
        clap_error_set(error, CLAP_ERR_TYPE_CONVERSION, "Float cannot be NaN or Infinity");
        return false;
    }

    *(double*)output = val;
    return true;
}

bool clap_type_bool_handler(const char *input, void *output,
                             size_t output_size, clap_error_t *error) {
    if (output_size != sizeof(bool)) {
        clap_error_set(error, CLAP_ERR_TYPE_CONVERSION, "Invalid output size");
        return false;
    }

    if (strcmp(input, "true") == 0 || strcmp(input, "yes") == 0 ||
        strcmp(input, "1") == 0 || strcmp(input, "on") == 0) {
        *(bool*)output = true;
        return true;
    }

    if (strcmp(input, "false") == 0 || strcmp(input, "no") == 0 ||
        strcmp(input, "0") == 0 || strcmp(input, "off") == 0) {
        *(bool*)output = false;
        return true;
    }

    clap_error_set(error, CLAP_ERR_TYPE_CONVERSION,
                   "Invalid boolean: '%s' (use true/false, yes/no, 1/0)", input);
    return false;
}

bool clap_register_builtin_types(clap_parser_t *parser) {
    if (!parser) return false;
    clap_register_type(parser, "string", clap_type_string_handler, sizeof(char*));
    clap_register_type(parser, "int", clap_type_int_handler, sizeof(int));
    clap_register_type(parser, "float", clap_type_float_handler, sizeof(double));
    clap_register_type(parser, "bool", clap_type_bool_handler, sizeof(bool));
    return true;
}

/* Look up a type handler from the parser's type registry by name. */
static clap_type_handler_t find_type_handler(clap_parser_t *parser, const char *type_name) {
    for (size_t i = 0; i < parser->type_handler_count; i++) {
        if (strcmp(parser->type_handlers[i].name, type_name) == 0) {
            return parser->type_handlers[i].handler;
        }
    }
    return NULL;
}

bool clap_apply_defaults(clap_parser_t *parser, clap_namespace_t *ns, clap_error_t *error) {
    for (size_t i = 0; i < parser->arg_count; i++) {
        clap_argument_t *arg = parser->arguments[i];
        const char *dest = clap_buffer_cstr(arg->dest);

        if (arg->default_string) {
            const char *type_name = clap_buffer_cstr(arg->type_name);

            if (strcmp(type_name, "string") == 0) {
                clap_namespace_set_string(ns, dest,
                    clap_buffer_cstr(arg->default_string));

            } else if (strcmp(type_name, "int") == 0) {
                int int_val;
                clap_type_handler_t handler = arg->type_handler ?
                    arg->type_handler : find_type_handler(parser, "int");
                if (handler && handler(clap_buffer_cstr(arg->default_string),
                            &int_val, sizeof(int), error)) {
                    clap_namespace_set_int(ns, dest, int_val);
                } else {
                    return false;
                }

            } else if (strcmp(type_name, "float") == 0) {
                double float_val;
                clap_type_handler_t handler = arg->type_handler ?
                    arg->type_handler : find_type_handler(parser, "float");
                if (handler && handler(clap_buffer_cstr(arg->default_string),
                            &float_val, sizeof(double), error)) {
                    clap_namespace_set_float(ns, dest, float_val);
                } else {
                    return false;
                }

            } else if (strcmp(type_name, "bool") == 0) {
                bool bool_val;
                clap_type_handler_t handler = arg->type_handler ?
                    arg->type_handler : find_type_handler(parser, "bool");
                if (handler && handler(clap_buffer_cstr(arg->default_string),
                            &bool_val, sizeof(bool), error)) {
                    clap_namespace_set_bool(ns, dest, bool_val);
                } else {
                    return false;
                }

            } else {
                clap_namespace_set_string(ns, dest,
                    clap_buffer_cstr(arg->default_string));
            }

        } else if (arg->action == CLAP_ACTION_STORE_TRUE) {
            clap_namespace_set_bool(ns, dest, false);
        } else if (arg->action == CLAP_ACTION_STORE_FALSE) {
            clap_namespace_set_bool(ns, dest, true);
        } else if (arg->action == CLAP_ACTION_COUNT) {
            clap_namespace_set_int(ns, dest, 0);
        }
    }
    return true;
}
