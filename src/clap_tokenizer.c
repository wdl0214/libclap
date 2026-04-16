/**
 * @file clap_tokenizer.c
 * @brief Command line tokenization and parsing
 */

#include "clap_parser_internal.h"

static token_t tokenize_arg(const char *arg) {
    token_t token = {0};

    if (!arg) {
        token.type = TOKEN_END;
        return token;
    }

    size_t len = strlen(arg);

    /* Handle -- */
    if (len == 2 && arg[0] == '-' && arg[1] == '-') {
        token.type = TOKEN_STOP;
        return token;
    }

    /* Long options */
    if (len >= 3 && arg[0] == '-' && arg[1] == '-') {
        const char *eq = strchr(arg, '=');
        if (eq) {
            token.type = TOKEN_LONG_OPTION_EQ;
            token.option_name = arg + 2;  /* Points to start, caller must handle '=' */
            token.value = eq + 1;
        } else {
            token.type = TOKEN_LONG_OPTION;
            token.option_name = arg + 2;
        }
        return token;
    }

    /* Short options */
    if (len >= 2 && arg[0] == '-' && arg[1] != '-') {
        if (len == 2) {
            token.type = TOKEN_SHORT_OPTION;
            token.option_name = arg + 1;
        } else {
            token.type = TOKEN_SHORT_OPTION_BUNDLE;
            token.option_name = arg + 1;
        }
        return token;
    }

    /* Positional */
    token.type = TOKEN_POSITIONAL;
    token.value = arg;
    return token;
}

static char** expand_short_bundle(const char *bundle, size_t *count) {
    size_t len = strlen(bundle);
    *count = len;

    char **expanded = clap_calloc(len, sizeof(char*));
    if (!expanded) return NULL;

    for (size_t i = 0; i < len; i++) {
        expanded[i] = clap_malloc(3);
        if (!expanded[i]) {
            for (size_t j = 0; j < i; j++) {
                clap_free(expanded[j]);
            }
            clap_free(expanded);
            return NULL;
        }
        expanded[i][0] = '-';
        expanded[i][1] = bundle[i];
        expanded[i][2] = '\0';
    }

    return expanded;
}


/* Check for mutually exclusive group conflicts */
static bool check_mutex_conflict(clap_parser_t *parser,
                                  clap_argument_t *arg,
                                  bool *mutex_group_used,
                                  const char *option_str,
                                  clap_error_t *error) {
    (void)option_str;    
    if (arg->group_id < 0 || !mutex_group_used) {
        return true;
    }

    if (mutex_group_used[arg->group_id]) {
        const char *conflicting_opt = NULL;
        clap_mutex_group_t *group = parser->mutex_groups[arg->group_id];

        /* Find the conflicting option */
        for (size_t k = 0; k < group->arg_count; k++) {
            clap_argument_t *other = group->arguments[k];
            if (other != arg) {
                clap_buffer_t *opt_name = clap_buffer_empty();
                for (size_t m = 0; m < other->option_count; m++) {
                    if (m > 0) clap_buffer_cat(&opt_name, "/");
                    clap_buffer_cat(&opt_name, other->option_strings[m]);
                }
                conflicting_opt = clap_strdup(clap_buffer_cstr(opt_name));
                clap_buffer_free(opt_name);
                break;
            }
        }

        clap_buffer_t *current_opt = clap_buffer_empty();
        for (size_t m = 0; m < arg->option_count; m++) {
            if (m > 0) clap_buffer_cat(&current_opt, "/");
            clap_buffer_cat(&current_opt, arg->option_strings[m]);
        }

        clap_error_set(error, CLAP_ERR_MUTUALLY_EXCLUSIVE,
                       "argument %s: not allowed with argument %s",
                       clap_buffer_cstr(current_opt),
                       conflicting_opt ? conflicting_opt : "another option");

        clap_buffer_free(current_opt);
        if (conflicting_opt) {
            clap_free((void*)conflicting_opt);
        }

        return false;
    }

    mutex_group_used[arg->group_id] = true;
    return true;
}

/* Helper function to execute an action */
static bool execute_action(clap_parser_t *parser,
                           clap_argument_t *arg,
                           clap_namespace_t *ns,
                           const char *value,
                           clap_error_t *error) {
    switch (arg->action) {
    case CLAP_ACTION_STORE:
        if (value) {
            return clap_namespace_set_string(ns, clap_buffer_cstr(arg->dest), value);
        }
        return true;
        
    case CLAP_ACTION_STORE_CONST:
        if (!arg->const_value) {
            clap_error_set(error, CLAP_ERR_INVALID_ARGUMENT,
                           "STORE_CONST action requires const value");
            return false;
        }
        return clap_namespace_set_string(ns, clap_buffer_cstr(arg->dest),
                                          clap_buffer_cstr(arg->const_value));
                                          
    case CLAP_ACTION_STORE_TRUE:
        return clap_namespace_set_bool(ns, clap_buffer_cstr(arg->dest), true);
        
    case CLAP_ACTION_STORE_FALSE:
        return clap_namespace_set_bool(ns, clap_buffer_cstr(arg->dest), false);
        
    case CLAP_ACTION_APPEND:
        if (value) {
            return clap_namespace_append_string(ns, clap_buffer_cstr(arg->dest), value);
        }
        return true;
        
    case CLAP_ACTION_APPEND_CONST:
        if (!arg->const_value) {
            clap_error_set(error, CLAP_ERR_INVALID_ARGUMENT,
                           "APPEND_CONST action requires const value");
            return false;
        }
        return clap_namespace_append_string(ns, clap_buffer_cstr(arg->dest),
                                             clap_buffer_cstr(arg->const_value));
                                             
    case CLAP_ACTION_COUNT: {
        int count = 0;
        clap_namespace_get_int(ns, clap_buffer_cstr(arg->dest), &count);
        return clap_namespace_set_int(ns, clap_buffer_cstr(arg->dest), count + 1);
    }
    
    case CLAP_ACTION_CUSTOM:
        if (!arg->action_handler) {
            clap_error_set(error, CLAP_ERR_INVALID_ARGUMENT,
                           "CUSTOM action requires action_handler");
            return false;
        }
        {
            const char *values[1] = { value };
            size_t count = value ? 1 : 0;
            return arg->action_handler(parser, arg, ns, values, count,
                                        arg->action_data, error);
        }
        
    default:
        return true;
    }
}

/* Check required optional arguments */
static bool check_required_option(clap_argument_t *arg,
                                   clap_namespace_t *ns,
                                   clap_error_t *error) {
    if (!(arg->flags & CLAP_ARG_REQUIRED)) {
        return true;
    }

    bool is_present = false;
    
    if (arg->action == CLAP_ACTION_STORE_TRUE || 
        arg->action == CLAP_ACTION_STORE_FALSE) {
        bool value;
        is_present = clap_namespace_get_bool(ns, clap_buffer_cstr(arg->dest), &value);
    } else if (arg->action == CLAP_ACTION_COUNT) {
        int count;
        is_present = clap_namespace_get_int(ns, clap_buffer_cstr(arg->dest), &count) && count > 0;
    } else {
        const char *value;
        is_present = clap_namespace_get_string(ns, clap_buffer_cstr(arg->dest), &value);
    }

    if (!is_present) {
        clap_buffer_t *opt_name = clap_buffer_empty();
        for (size_t j = 0; j < arg->option_count; j++) {
            if (j > 0) clap_buffer_cat(&opt_name, "/");
            clap_buffer_cat(&opt_name, arg->option_strings[j]);
        }
        
        clap_error_set(error, CLAP_ERR_REQUIRED_MISSING,
                       "the following arguments are required: %s",
                       clap_buffer_cstr(opt_name));
        
        clap_buffer_free(opt_name);
        return false;
    }
    
    return true;
}

bool clap_parse_args(clap_parser_t *parser,
                     int argc,
                     char *argv[],
                     clap_namespace_t **out_ns,
                     clap_error_t *error) {
    if (!parser || !argv || !out_ns) {
        clap_error_set(error, CLAP_ERR_INVALID_ARGUMENT, "Invalid parameters");
        return false;
    }

    /* Create namespace */
    clap_namespace_t *ns = clap_namespace_new();
    if (!ns) {
        clap_error_set(error, CLAP_ERR_MEMORY, "Failed to create namespace");
        return false;
    }

    /* Apply defaults */
    clap_apply_defaults(parser, ns, error);

    /* Track which mutex groups have been used */
    bool *mutex_group_used = NULL;
    if (parser->mutex_group_count > 0) {
        mutex_group_used = clap_calloc(parser->mutex_group_count, sizeof(bool));
        if (!mutex_group_used) {
            clap_error_set(error, CLAP_ERR_MEMORY, "Failed to allocate mutex tracking");
            clap_namespace_free(ns);
            return false;
        }
    }

    /* Parse arguments */
    int pos = 1;
    size_t positional_index = 0;
    bool parsing_options = true;

    while (pos < argc) {
        const char *current = argv[pos];
        token_t token = tokenize_arg(current);

        if (token.type == TOKEN_STOP) {
            parsing_options = false;
            pos++;
            continue;
        }

        if (parsing_options && token.type == TOKEN_SHORT_OPTION_BUNDLE) {
            size_t expanded_count = 0;
            char **expanded = expand_short_bundle(token.option_name, &expanded_count);
            if (!expanded) {
                clap_error_set(error, CLAP_ERR_MEMORY, "Failed to expand option bundle");
                clap_free(mutex_group_used);
                clap_namespace_free(ns);
                return false;
            }

            for (size_t i = 0; i < expanded_count; i++) {
                token_t sub_token = tokenize_arg(expanded[i]);
                clap_argument_t *arg = clap_find_option(parser, sub_token.option_name, false);

                if (!arg) {
                    clap_error_set(error, CLAP_ERR_UNRECOGNIZED,
                                   "Unrecognized option '%s'", expanded[i]);
                    for (size_t j = 0; j < expanded_count; j++) clap_free(expanded[j]);
                    clap_free(expanded);
                    clap_free(mutex_group_used);
                    clap_namespace_free(ns);
                    return false;
                }

                /* Check mutex group */
                if (!check_mutex_conflict(parser, arg, mutex_group_used, expanded[i], error)) {
                    for (size_t j = 0; j < expanded_count; j++) clap_free(expanded[j]);
                    clap_free(expanded);
                    clap_free(mutex_group_used);
                    clap_namespace_free(ns);
                    return false;
                }

                /* Handle help/version actions */
                if (arg->action == CLAP_ACTION_HELP) {
                    clap_free(mutex_group_used);
                    for (size_t j = 0; j < expanded_count; j++) clap_free(expanded[j]);
                    clap_free(expanded);
                    clap_print_help(parser, stdout);
                    exit(0);
                }
                if (arg->action == CLAP_ACTION_VERSION) {
                    clap_free(mutex_group_used);
                    for (size_t j = 0; j < expanded_count; j++) clap_free(expanded[j]);
                    clap_free(expanded);
                    clap_print_version(parser, stdout);
                    exit(0);
                }

                /* Execute the action (no value for bundled short options) */
                if (!execute_action(parser, arg, ns, NULL, error)) {
                    for (size_t j = 0; j < expanded_count; j++) clap_free(expanded[j]);
                    clap_free(expanded);
                    clap_free(mutex_group_used);
                    clap_namespace_free(ns);
                    return false;
                }
            }

            for (size_t j = 0; j < expanded_count; j++) clap_free(expanded[j]);
            clap_free(expanded);
            pos++;
            continue;
        }

        if (parsing_options && (token.type == TOKEN_LONG_OPTION ||
                                token.type == TOKEN_LONG_OPTION_EQ ||
                                token.type == TOKEN_SHORT_OPTION)) {

            bool is_long = (token.type != TOKEN_SHORT_OPTION);
            const char *lookup_name = token.option_name;
            
            /* For long options with '=', extract just the name part */
            if (token.type == TOKEN_LONG_OPTION_EQ) {
                const char *eq = strchr(token.option_name, '=');
                if (eq) {
                    size_t len = (size_t)(eq - token.option_name);
                    lookup_name = clap_strndup(token.option_name, len);
                }
            }
            
            clap_argument_t *arg = clap_find_option(parser, lookup_name, is_long);
            
            /* Free temporary string if allocated */
            if (token.type == TOKEN_LONG_OPTION_EQ && lookup_name != token.option_name) {
                clap_free((void*)lookup_name);
            }

            if (!arg) {
                clap_error_set(error, CLAP_ERR_UNRECOGNIZED,
                               "Unrecognized option '%s'", current);
                clap_free(mutex_group_used);
                clap_namespace_free(ns);
                return false;
            }

            /* Check mutex group */
            if (!check_mutex_conflict(parser, arg, mutex_group_used, current, error)) {
                clap_free(mutex_group_used);
                clap_namespace_free(ns);
                return false;
            }

            /* Handle help/version actions */
            if (arg->action == CLAP_ACTION_HELP) {
                clap_free(mutex_group_used);
                clap_print_help(parser, stdout);
                exit(0);
            }
            if (arg->action == CLAP_ACTION_VERSION) {
                clap_free(mutex_group_used);
                clap_print_version(parser, stdout);
                exit(0);
            }

            /* Get value(s) */
            const char *value = NULL;
            if (token.type == TOKEN_LONG_OPTION_EQ) {
                value = token.value;
            } else if (arg->nargs != 0 && 
                       arg->action != CLAP_ACTION_STORE_TRUE &&
                       arg->action != CLAP_ACTION_STORE_FALSE &&
                       arg->action != CLAP_ACTION_STORE_CONST &&
                       arg->action != CLAP_ACTION_APPEND_CONST &&
                       arg->action != CLAP_ACTION_COUNT) {
                if (pos + 1 < argc && argv[pos + 1][0] != '-') {
                    value = argv[++pos];
                }
            }

            /* Execute the action */
            if (!execute_action(parser, arg, ns, value, error)) {
                clap_free(mutex_group_used);
                clap_namespace_free(ns);
                return false;
            }

            pos++;
        } else {
            /* Not an option - could be subcommand or positional */

            /* First check if this is a subcommand */
            if (parser->has_subparsers && positional_index == 0) {
                clap_parser_t *subparser = NULL;
                
                if (parser->subparsers_container) {
                    for (size_t i = 0; i < parser->subparsers_container->subparser_count; i++) {
                        clap_parser_t *sub = parser->subparsers_container->subparsers[i];
                        
                        const char *full_name = clap_buffer_cstr(sub->prog_name);
                        const char *cmd_name = strrchr(full_name, ' ');
                        if (cmd_name) {
                            cmd_name++;
                        } else {
                            cmd_name = full_name;
                        }

                        if (strcmp(cmd_name, current) == 0) {
                            subparser = sub;
                            break;
                        }
                    }
                }

                if (subparser) {
                    clap_namespace_t *sub_ns = NULL;
                    clap_error_t sub_error = {0};

                    if (!clap_parse_args(subparser, argc - pos, argv + pos, &sub_ns, &sub_error)) {
                        /* Subcommand parsing failed - return error to caller */
                        clap_free(mutex_group_used);
                        clap_namespace_free(ns);
                        
                        /* Copy error to output */
                        *error = sub_error;
                        error->subcommand_name = current;
                        
                        /* Store subparser in error context for caller to print help */
                        /* Caller can check error code and print subcommand help */
                        return false;
                    }
                    clap_namespace_set_string(ns, clap_buffer_cstr(parser->subparser_dest), current);

                    clap_namespace_merge(ns, sub_ns);

                    clap_namespace_free(sub_ns);

                    clap_free(mutex_group_used);
                    *out_ns = ns;
                    return true;
                }
            }

            /* Not a subcommand - treat as positional argument */
            if (positional_index >= parser->positional_count) {
                clap_error_set(error, CLAP_ERR_TOO_MANY_ARGS,
                               "Unexpected argument '%s'", current);
                clap_free(mutex_group_used);
                clap_namespace_free(ns);
                return false;
            }

            clap_argument_t *pos_arg = parser->positional_args[positional_index];
            
            /* Execute action for positional argument */
            if (!execute_action(parser, pos_arg, ns, current, error)) {
                clap_free(mutex_group_used);
                clap_namespace_free(ns);
                return false;
            }

            pos++;
            if (!(pos_arg->flags & CLAP_ARG_MULTIPLE)) {
                positional_index++;
            }
        }
    }

    /* Check required positional arguments */
    for (size_t i = 0; i < parser->positional_count; i++) {
        clap_argument_t *arg = parser->positional_args[i];
        if ((arg->flags & CLAP_ARG_REQUIRED)) {
            const char *value;
            if (!clap_namespace_get_string(ns, clap_buffer_cstr(arg->dest), &value)) {
                clap_error_set(error, CLAP_ERR_REQUIRED_MISSING,
                               "the following arguments are required: %s",
                               clap_buffer_cstr(arg->display_name));
                clap_free(mutex_group_used);
                clap_namespace_free(ns);
                return false;
            }
        }
    }

    /* Check required optional arguments */
    for (size_t i = 0; i < parser->optional_count; i++) {
        clap_argument_t *arg = parser->optional_args[i];
        if (!check_required_option(arg, ns, error)) {
            clap_free(mutex_group_used);
            clap_namespace_free(ns);
            return false;
        }
    }

    /* Check required mutex groups */
    for (size_t i = 0; i < parser->mutex_group_count; i++) {
        clap_mutex_group_t *group = parser->mutex_groups[i];
        if (group->required && !mutex_group_used[i]) {
            clap_buffer_t *opts = clap_buffer_empty();
            for (size_t j = 0; j < group->arg_count; j++) {
                if (j > 0) clap_buffer_cat(&opts, " ");
                clap_argument_t *arg = group->arguments[j];
                clap_buffer_cat(&opts, arg->option_strings[0]);
            }

            clap_error_set(error, CLAP_ERR_REQUIRED_MISSING,
                           "one of the arguments %s is required",
                           clap_buffer_cstr(opts));

            clap_buffer_free(opts);
            clap_free(mutex_group_used);
            clap_namespace_free(ns);
            return false;
        }
    }

    clap_free(mutex_group_used);
    *out_ns = ns;
    return true;
}