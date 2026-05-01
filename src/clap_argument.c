/**
 * @file clap_argument.c
 * @brief Argument definition implementation
 */

#include "clap_parser_internal.h"

static bool parse_name_or_flags(clap_argument_t *arg, const char *spec, clap_error_t *error) {
    if (!spec || !*spec) {
        clap_error_set(error, CLAP_ERR_INVALID_ARGUMENT, "Empty argument name");
        return false;
    }
    
    /* Positional argument (no leading dash) */
    if (spec[0] != '-') {
        arg->flags |= CLAP_ARG_POSITIONAL;
        arg->flags |= CLAP_ARG_REQUIRED;
        arg->option_strings = clap_calloc(1, sizeof(char*));
        if (!arg->option_strings) {
            clap_error_set(error, CLAP_ERR_MEMORY, "Failed to allocate memory");
            return false;
        }
        arg->option_strings[0] = clap_strdup(spec);
        arg->option_count = 1;
        arg->display_name = clap_buffer_new(spec);
        arg->dest = clap_buffer_new(spec);
        return true;
    }
    
    /* Optional argument - parse by splitting on '/' */
    arg->flags |= CLAP_ARG_OPTIONAL;
    
    /* Count options by counting '/' + 1 */
    size_t count = 1;
    for (const char *p = spec; *p; p++) {
        if (*p == '/') count++;
    }
    
    arg->option_strings = clap_calloc(count, sizeof(char*));
    if (!arg->option_strings) {
        clap_error_set(error, CLAP_ERR_MEMORY, "Failed to allocate memory");
        return false;
    }
    arg->option_count = 0;
    
    /* Parse by splitting on '/' */
    const char *start = spec;
    const char *p = spec;
    
    while (*p) {
        if (*p == '/') {
            size_t len = (size_t)(p - start);
            if (len > 0) {
                char *opt = clap_strndup(start, len);
                if (!opt) {
                    clap_error_set(error, CLAP_ERR_MEMORY, "Failed to allocate memory");
                    return false;
                }
                arg->option_strings[arg->option_count++] = opt;
            }
            start = p + 1;
        }
        p++;
    }
    
    /* Extract the last option */
    if (*start) {
        char *opt = clap_strdup(start);
        if (!opt) {
            clap_error_set(error, CLAP_ERR_MEMORY, "Failed to allocate memory");
            return false;
        }
        arg->option_strings[arg->option_count++] = opt;
    }
    
    if (arg->option_count == 0) {
        clap_error_set(error, CLAP_ERR_INVALID_ARGUMENT, "No valid options found");
        return false;
    }
    
    /* Use longest option as display name */
    const char *longest = arg->option_strings[0];
    for (size_t i = 1; i < arg->option_count; i++) {
        if (strlen(arg->option_strings[i]) > strlen(longest)) {
            longest = arg->option_strings[i];
        }
    }
    arg->display_name = clap_buffer_new(longest);
    
    /* Generate dest from longest option */
    const char *dest_start = longest;
    while (*dest_start == '-') dest_start++;
    
    clap_buffer_t *dest = clap_buffer_new(dest_start);
    char *data = (char*)clap_buffer_cstr(dest);
    for (size_t i = 0; data[i]; i++) {
        if (data[i] == '-') data[i] = '_';
    }
    arg->dest = dest;
    
    return true;
}

clap_argument_t* clap_add_argument(clap_parser_t *parser, const char *name_or_flags) {
    if (!parser || !name_or_flags) return NULL;

    /* Expand arguments array */
    if (parser->arg_count >= parser->arg_capacity) {
        size_t new_cap = parser->arg_capacity * 2;
        if (new_cap == 0) new_cap = 8;
        clap_argument_t **new_args = clap_realloc(
            parser->arguments, new_cap * sizeof(clap_argument_t*)
        );
        if (!new_args) return NULL;
        parser->arguments = new_args;
        parser->arg_capacity = new_cap;
    }

    /* Create argument */
    clap_argument_t *arg = clap_calloc(1, sizeof(clap_argument_t));
    if (!arg) return NULL;

    arg->nargs = 1;
    arg->mutex_group_id = CLAP_MUTEX_GROUP_NONE;
    arg->display_group_id = CLAP_DISPLAY_GROUP_NONE;
    arg->action = CLAP_ACTION_STORE;
    arg->type_name = clap_buffer_new("string");

    clap_error_t error = {0};
    if (!parse_name_or_flags(arg, name_or_flags, &error)) {
        clap_free(arg);
        parser->last_error = error;
        return NULL;
    }

    parser->arguments[parser->arg_count++] = arg;

    /* Index by type */
    if (arg->flags & CLAP_ARG_POSITIONAL) {
        arg->position = (int)parser->positional_count;
        clap_argument_t **new_pos = clap_realloc(
            parser->positional_args,
            (parser->positional_count + 1) * sizeof(clap_argument_t*)
        );
        if (!new_pos) {
            return arg;  /* Still return arg, just not indexed */
        }
        parser->positional_args = new_pos;
        parser->positional_args[parser->positional_count++] = arg;
    } else {
        clap_argument_t **new_opt = clap_realloc(
            parser->optional_args,
            (parser->optional_count + 1) * sizeof(clap_argument_t*)
        );
        if (!new_opt) {
            return arg;  /* Still return arg, just not indexed */
        }
        parser->optional_args = new_opt;
        parser->optional_args[parser->optional_count++] = arg;
    }

    return arg;
}

clap_argument_t* clap_argument_help(clap_argument_t *arg, const char *help_text) {
    if (arg && help_text) {
        clap_buffer_free(arg->help_text);
        arg->help_text = clap_buffer_new(help_text);
    }
    return arg;
}

clap_argument_t* clap_argument_type(clap_argument_t *arg, const char *type_name) {
    if (arg && type_name) {
        clap_buffer_free(arg->type_name);
        arg->type_name = clap_buffer_new(type_name);
    }
    return arg;
}

clap_argument_t* clap_argument_default(clap_argument_t *arg, const char *default_value) {
    if (arg && default_value) {
        clap_buffer_free(arg->default_string);
        arg->default_string = clap_buffer_new(default_value);
    }
    return arg;
}

clap_argument_t* clap_argument_required(clap_argument_t *arg, bool required) {
    if (arg) {
        if (arg->flags & CLAP_ARG_POSITIONAL) {
            /* Positional arguments cannot have required explicitly set */
            return arg;
        }

        if (required) {
            arg->flags |= CLAP_ARG_REQUIRED;
        } else {
            arg->flags &= ~((unsigned int)CLAP_ARG_REQUIRED);
        }
    }
    return arg;
}

clap_argument_t* clap_argument_choices(clap_argument_t *arg, const char **choices, size_t count) {
    if (arg && choices && count > 0) {
        for (size_t i = 0; i < arg->choice_count; i++) {
            clap_free(arg->choices[i]);
        }
        clap_free(arg->choices);
        
        arg->choices = clap_calloc(count, sizeof(char*));
        if (arg->choices) {
            arg->choice_count = count;
            for (size_t i = 0; i < count; i++) {
                arg->choices[i] = clap_strdup(choices[i]);
            }
        }
    }
    return arg;
}

clap_argument_t* clap_argument_const(clap_argument_t *arg, const char *const_value) {
    if (arg && const_value) {
        clap_buffer_free(arg->const_value);
        arg->const_value = clap_buffer_new(const_value);
    }
    return arg;
}

clap_argument_t* clap_argument_nargs(clap_argument_t *arg, int nargs) {
    if (arg) {
        /* Convert character nargs to internal constants */
        if (nargs == '*') {
            arg->nargs = CLAP_NARGS_ZERO_OR_MORE;
        } else if (nargs == '+') {
            arg->nargs = CLAP_NARGS_ONE_OR_MORE;
        } else if (nargs == '?') {
            arg->nargs = CLAP_NARGS_ZERO_OR_ONE;
        } else {
            arg->nargs = nargs;
        }

        /* Update CLAP_ARG_MULTIPLE flag and convert STORE to APPEND for nargs > 1 */
        if (arg->nargs == CLAP_NARGS_ZERO_OR_MORE ||
            arg->nargs == CLAP_NARGS_ONE_OR_MORE ||
            (arg->nargs > 1)) {
            arg->flags |= CLAP_ARG_MULTIPLE;

            /* For fixed nargs > 1, automatically use APPEND action if STORE is set */
            if (arg->nargs > 1 && arg->action == CLAP_ACTION_STORE) {
                arg->action = CLAP_ACTION_APPEND;
            }
        } else {
            arg->flags &= ~((unsigned int)CLAP_ARG_MULTIPLE);
        }

        /* Update CLAP_ARG_REQUIRED flag for positional arguments */
        if (arg->flags & CLAP_ARG_POSITIONAL) {
            if (arg->nargs == CLAP_NARGS_ZERO_OR_ONE ||
                arg->nargs == CLAP_NARGS_ZERO_OR_MORE ||
                arg->nargs == CLAP_NARGS_REMAINDER) {
                /* Optional positionals: '?', '*', REMAINDER */
                arg->flags &= ~((unsigned int)CLAP_ARG_REQUIRED);
                } else {
                    /* Required positionals: default (1), N, '+' */
                    arg->flags |= CLAP_ARG_REQUIRED;
                }
        }
    }
    return arg;
}

clap_argument_t* clap_argument_action(clap_argument_t *arg, clap_action_t action) {
    if (arg) {
        arg->action = action;
    }
    return arg;
}

clap_argument_t* clap_argument_metavar(clap_argument_t *arg, const char *metavar) {
    if (arg && metavar) {
        clap_buffer_free(arg->metavar);
        arg->metavar = clap_buffer_new(metavar);
    }
    return arg;
}

clap_argument_t* clap_argument_mutex_group(clap_argument_t *arg, int mutex_group_id) {
    if (arg) {
        arg->mutex_group_id = mutex_group_id;
    }
    return arg;
}

clap_argument_t* clap_argument_dest(clap_argument_t *arg, const char *dest) {
    if (arg && dest) {
        clap_buffer_free(arg->dest);
        arg->dest = clap_buffer_new(dest);
    }
    return arg;
}

clap_argument_t* clap_argument_handler(clap_argument_t *arg, clap_action_handler_t handler) {
    if (arg) {
        arg->action_handler = handler;
    }
    return arg;
}
