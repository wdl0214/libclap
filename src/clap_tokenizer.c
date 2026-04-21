/**
 * @file clap_tokenizer.c
 * @brief Command line tokenization and parsing
 */

#include "clap_parser_internal.h"


clap_token_t tokenize_arg(const char *arg) {
    clap_token_t token = {0};
    token.raw = arg;
    token.name_allocated = false;

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
            size_t name_len = (size_t)(eq - arg - 2);
            token.option_name = clap_strndup(arg + 2, name_len);
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

clap_token_t* clap_tokenize(int argc, char *argv[], size_t *count, clap_error_t *error) {
    if (!argv || argc < 1 || !count) {
        if (error) {
            clap_error_set(error, CLAP_ERR_INVALID_ARGUMENT, "Invalid tokenization parameters");
        }
        return NULL;
    }

    size_t token_count = argc > 1 ? (size_t)(argc - 1) : 0;
    *count = token_count;

    if (token_count == 0) {
        return NULL;
    }

    clap_token_t *tokens = clap_calloc(token_count, sizeof(clap_token_t));
    if (!tokens) {
        if (error) {
            clap_error_set(error, CLAP_ERR_MEMORY, "Failed to allocate tokens");
        }
        return NULL;
    }

    for (int i = 1; i < argc; i++) {
        tokens[i - 1] = tokenize_arg(argv[i]);
    }

    return tokens;
}

void clap_tokenize_free(clap_token_t *tokens, size_t count) {
    if (!tokens) return;

    for (size_t i = 0; i < count; i++) {
        if (tokens[i].name_allocated && tokens[i].option_name) {
            clap_free((void*)tokens[i].option_name);
        }
    }
    clap_free(tokens);
}

char** clap_expand_short_bundle(const char *bundle, size_t *count) {
    size_t len = strlen(bundle);
    *count = len;

    char **expanded = clap_malloc(len * sizeof(char*) + sizeof(char*));
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

bool check_required_positional(clap_argument_t *arg, clap_namespace_t *ns, clap_error_t *error) {
    /* Positional arguments are required unless nargs='?' or '*' or REMAINDER */
    if (arg->nargs == CLAP_NARGS_ZERO_OR_ONE ||
        arg->nargs == CLAP_NARGS_ZERO_OR_MORE ||
        arg->nargs == CLAP_NARGS_REMAINDER) {
        return true;
    }

    bool is_present = false;

    if (arg->flags & CLAP_ARG_MULTIPLE) {
        const char **values;
        size_t count = 0;
        is_present = clap_namespace_get_string_array(ns, clap_buffer_cstr(arg->dest),
                                                      &values, &count) && count > 0;
    } else if (arg->nargs > 1) {
        /* nargs=N (N>1) - check array */
        const char **values;
        size_t count = 0;
        is_present = clap_namespace_get_string_array(ns, clap_buffer_cstr(arg->dest),
                                                      &values, &count) && count == (size_t)arg->nargs;
    } else {
        const char *value;
        is_present = clap_namespace_get_string(ns, clap_buffer_cstr(arg->dest), &value);
    }

    if (!is_present) {
        clap_error_set(error, CLAP_ERR_REQUIRED_MISSING,
                       "the following arguments are required: %s",
                       clap_buffer_cstr(arg->display_name));
        return false;
    }

    return true;
}

bool check_required_option(clap_argument_t *arg, clap_namespace_t *ns, clap_error_t *error) {
    if (!(arg->flags & CLAP_ARG_REQUIRED)) {
        return true;
    }

    bool is_present = false;

    if (arg->flags & CLAP_ARG_MULTIPLE) {
        /* nargs='+' or '*' - check array */
        const char **values;
        size_t count = 0;
        is_present = clap_namespace_get_string_array(ns, clap_buffer_cstr(arg->dest),
                                                      &values, &count) && count > 0;
    } else if (arg->action == CLAP_ACTION_STORE_TRUE ||
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

bool check_positional_nargs_constraint(clap_argument_t *arg, clap_namespace_t *ns, clap_error_t *error) {
    if (arg->nargs == CLAP_NARGS_ONE_OR_MORE) {
        const char **values;
        size_t count = 0;
        if (!clap_namespace_get_string_array(ns, clap_buffer_cstr(arg->dest), &values, &count) || count < 1) {
            clap_error_set(error, CLAP_ERR_REQUIRED_MISSING,
                           "the following arguments are required: %s",
                           clap_buffer_cstr(arg->display_name));
            return false;
        }
    }

    if (arg->nargs > 1) {
        const char **values;
        size_t count = 0;
        if (!clap_namespace_get_string_array(ns, clap_buffer_cstr(arg->dest), &values, &count)) {
            count = 0;
        }
        if (count != (size_t)arg->nargs) {
            clap_error_set(error,
                count < (size_t)arg->nargs ? CLAP_ERR_TOO_FEW_ARGS : CLAP_ERR_TOO_MANY_ARGS,
                "Expected %d argument(s), got %zu", arg->nargs, count);
            return false;
        }
    }

    return true;
}

bool clap_parse_args(clap_parser_t *parser, int argc, char *argv[],
                     clap_namespace_t **out_ns,
                     clap_error_t *error) {
    if (!parser || !argv || !out_ns) {
        clap_error_set(error, CLAP_ERR_INVALID_ARGUMENT, "Invalid parameters");
        return false;
    }

    clap_namespace_t *ns = clap_namespace_new();
    if (!ns) {
        clap_error_set(error, CLAP_ERR_MEMORY, "Failed to create namespace");
        return false;
    }

    if (!clap_apply_defaults(parser, ns, error)) {
        clap_namespace_free(ns);
        return false;
    }

    bool *mutex_group_used = NULL;
    if (parser->mutex_group_count > 0) {
        mutex_group_used = clap_calloc(parser->mutex_group_count, sizeof(bool));
        if (!mutex_group_used) {
            clap_error_set(error, CLAP_ERR_MEMORY, "Failed to allocate mutex tracking");
            clap_namespace_free(ns);
            return false;
        }
    }

    size_t token_count = 0;
    clap_token_t *tokens = clap_tokenize(argc, argv, &token_count, error);
    if (!tokens && token_count > 0) {
        clap_free(mutex_group_used);
        clap_namespace_free(ns);
        return false;
    }

    clap_pattern_t *pattern = clap_analyze_pattern(parser, tokens, token_count, error);
    if (!pattern) {
        clap_tokenize_free(tokens, token_count);
        clap_free(mutex_group_used);
        clap_namespace_free(ns);
        return false;
    }

    if (!clap_parse_with_pattern(parser, tokens, pattern, ns, mutex_group_used, error)) {
        clap_pattern_free(pattern);
        clap_tokenize_free(tokens, token_count);
        clap_free(mutex_group_used);
        clap_namespace_free(ns);
        return false;
    }

    clap_pattern_free(pattern);
    clap_tokenize_free(tokens, token_count);

    /* Check required positional arguments */
    for (size_t i = 0; i < parser->positional_count; i++) {
        if (!check_required_positional(parser->positional_args[i], ns, error)) {
            clap_free(mutex_group_used);
            clap_namespace_free(ns);
            return false;
        }
    }

    /* Check required optional arguments */
    for (size_t i = 0; i < parser->optional_count; i++) {
        if (!check_required_option(parser->optional_args[i], ns, error)) {
            clap_free(mutex_group_used);
            clap_namespace_free(ns);
            return false;
        }
    }

    /* Check nargs constraints for positional arguments */
    for (size_t i = 0; i < parser->positional_count; i++) {
        if (!check_positional_nargs_constraint(parser->positional_args[i], ns, error)) {
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
