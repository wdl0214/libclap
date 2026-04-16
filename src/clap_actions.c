/**
 * @file clap_actions.c
 * @brief Action handlers implementation
 */

#include "clap_parser_internal.h"

/* STORE action */
bool clap_action_store(clap_parser_t *parser,
                       clap_argument_t *arg,
                       clap_namespace_t *ns,
                       const char **values,
                       size_t count,
                       void *user_data,
                       clap_error_t *error) {
    (void)parser;
    (void)user_data;
    (void)error;
    
    if (count == 0) return true;
    return clap_namespace_set_string(ns, clap_buffer_cstr(arg->dest), values[0]);
}

/* STORE_CONST action */
bool clap_action_store_const(clap_parser_t *parser,
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
                       "STORE_CONST action requires const value");
        return false;
    }
    
    return clap_namespace_set_string(ns, clap_buffer_cstr(arg->dest),
                                      clap_buffer_cstr(arg->const_value));
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