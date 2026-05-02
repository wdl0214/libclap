/**
 * @file clap_actions.c
 * @brief Action handlers implementation
 */

#include "clap_parser_internal.h"

static bool resolve_type_handler(clap_parser_t *parser, clap_argument_t *arg) {
    if (arg->type_handler || !parser) return false;

    const char *type_name = clap_buffer_cstr(arg->type_name);
    for (size_t i = 0; i < parser->type_handler_count; i++) {
        if (strcmp(parser->type_handlers[i].name, type_name) == 0) {
            arg->type_handler = parser->type_handlers[i].handler;
            arg->type_size = parser->type_handlers[i].size;
            return true;
        }
    }
    return false;
}

static bool convert_and_store(clap_parser_t *parser,
                              clap_argument_t *arg,
                              clap_namespace_t *ns,
                              const char *value,
                              clap_error_t *error) {
    const char *type_name = clap_buffer_cstr(arg->type_name);

    if (strcmp(type_name, "string") == 0) {
        return clap_namespace_set_string(ns, clap_buffer_cstr(arg->dest), value);
    }

    if (strcmp(type_name, "int") == 0) {
        int int_val;
        if (!arg->type_handler) {
            arg->type_handler = clap_type_int_handler;
        }
        if (!arg->type_handler(value, &int_val, sizeof(int), error)) {
            return false;
        }
        return clap_namespace_set_int(ns, clap_buffer_cstr(arg->dest), int_val);
    }

    if (strcmp(type_name, "float") == 0) {
        double float_val;
        if (!arg->type_handler) {
            arg->type_handler = clap_type_float_handler;
        }
        if (!arg->type_handler(value, &float_val, sizeof(double), error)) {
            return false;
        }
        return clap_namespace_set_float(ns, clap_buffer_cstr(arg->dest), float_val);
    }

    if (strcmp(type_name, "bool") == 0) {
        bool bool_val;
        if (!arg->type_handler) {
            arg->type_handler = clap_type_bool_handler;
        }
        if (!arg->type_handler(value, &bool_val, sizeof(bool), error)) {
            return false;
        }
        return clap_namespace_set_bool(ns, clap_buffer_cstr(arg->dest), bool_val);
    }

    /* Resolve custom type handler from parser's registry */
    if (!arg->type_handler) {
        resolve_type_handler(parser, arg);
    }

    if (arg->type_handler) {
        void *buffer = clap_malloc(arg->type_size);
        if (!buffer) {
            clap_error_set(error, CLAP_ERR_MEMORY,
                           "Failed to allocate memory for type conversion");
            return false;
        }

        bool result = arg->type_handler(value, buffer, arg->type_size, error);
        clap_free(buffer);

        if (result) {
            return clap_namespace_set_string(ns, clap_buffer_cstr(arg->dest), value);
        }
        return false;
    }

    return clap_namespace_set_string(ns, clap_buffer_cstr(arg->dest), value);
}

/* STORE action */
bool clap_action_store(clap_parser_t *parser,
                       clap_argument_t *arg,
                       clap_namespace_t *ns,
                       const char **values,
                       size_t count,
                       void *user_data,
                       clap_error_t *error) {
    (void)user_data;

    if (count == 0) return true;
    return convert_and_store(parser, arg, ns, values[0], error);
}

/* STORE_CONST action */
bool clap_action_store_const(clap_parser_t *parser,
                              clap_argument_t *arg,
                              clap_namespace_t *ns,
                              const char **values,
                              size_t count,
                              void *user_data,
                              clap_error_t *error) {
    (void)values;
    (void)count;
    (void)user_data;

    if (!arg->const_value) {
        clap_error_set(error, CLAP_ERR_INVALID_ARGUMENT,
                       "STORE_CONST action requires const value");
        return false;
    }

    return convert_and_store(parser, arg, ns, clap_buffer_cstr(arg->const_value), error);
}

/* STORE_TRUE action */
bool clap_action_store_true(clap_parser_t *parser,
                            clap_argument_t *arg,
                            clap_namespace_t *ns,
                            const char **values,
                            size_t count,
                            void *user_data,
                            clap_error_t *error) {
    (void)parser;
    (void)values;
    (void)count;
    (void)user_data;
    (void)error;
    
    return clap_namespace_set_bool(ns, clap_buffer_cstr(arg->dest), true);
}

/* STORE_FALSE action */
bool clap_action_store_false(clap_parser_t *parser,
                             clap_argument_t *arg,
                             clap_namespace_t *ns,
                             const char **values,
                             size_t count,
                             void *user_data,
                             clap_error_t *error) {
    (void)parser;
    (void)values;
    (void)count;
    (void)user_data;
    (void)error;
    
    return clap_namespace_set_bool(ns, clap_buffer_cstr(arg->dest), false);
}

/* APPEND action */
bool clap_action_append(clap_parser_t *parser,
                        clap_argument_t *arg,
                        clap_namespace_t *ns,
                        const char **values,
                        size_t count,
                        void *user_data,
                        clap_error_t *error) {
    (void)parser;
    (void)user_data;
    (void)error;
    
    for (size_t i = 0; i < count; i++) {
        if (!clap_namespace_append_string(ns, clap_buffer_cstr(arg->dest), values[i])) {
            return false;
        }
    }
    return true;
}

/* APPEND_CONST action */
bool clap_action_append_const(clap_parser_t *parser,
                               clap_argument_t *arg,
                               clap_namespace_t *ns,
                               const char **values,
                               size_t count,
                               void *user_data,
                               clap_error_t *error) {
    (void)parser;
    (void)values;
    (void)count;
    (void)user_data;
    (void)error;
    
    if (!arg->const_value) {
        clap_error_set(error, CLAP_ERR_INVALID_ARGUMENT,
                       "APPEND_CONST action requires const value");
        return false;
    }
    
    return clap_namespace_append_string(ns, clap_buffer_cstr(arg->dest),
                                         clap_buffer_cstr(arg->const_value));
}

/* COUNT action */
bool clap_action_count(clap_parser_t *parser,
                       clap_argument_t *arg,
                       clap_namespace_t *ns,
                       const char **values,
                       size_t count,
                       void *user_data,
                       clap_error_t *error) {
    (void)parser;
    (void)values;
    (void)count;
    (void)user_data;
    (void)error;
    
    int current = 0;
    clap_namespace_get_int(ns, clap_buffer_cstr(arg->dest), &current);
    return clap_namespace_set_int(ns, clap_buffer_cstr(arg->dest), current + 1);
}

/* CUSTOM action */
bool clap_action_custom(clap_parser_t *parser,
                         clap_argument_t *arg,
                         clap_namespace_t *ns,
                         const char **values,
                         size_t count,
                         void *user_data,
                         clap_error_t *error) {
    if (!arg->action_handler) {
        clap_error_set(error, CLAP_ERR_INVALID_ARGUMENT,
                       "CUSTOM action requires action_handler");
        return false;
    }
    
    return arg->action_handler(parser, arg, ns, values, count, user_data, error);
}

/* Action dispatcher */
clap_action_handler_t get_action_handler(clap_action_t action) {
    switch (action) {
    case CLAP_ACTION_STORE:
        return clap_action_store;
    case CLAP_ACTION_STORE_CONST:
        return clap_action_store_const;
    case CLAP_ACTION_STORE_TRUE:
        return clap_action_store_true;
    case CLAP_ACTION_STORE_FALSE:
        return clap_action_store_false;
    case CLAP_ACTION_APPEND:
        return clap_action_append;
    case CLAP_ACTION_APPEND_CONST:
        return clap_action_append_const;
    case CLAP_ACTION_COUNT:
        return clap_action_count;
    case CLAP_ACTION_CUSTOM:
        return clap_action_custom;
    default:
        return NULL;
    }
}
