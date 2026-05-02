/**
 * @file clap_parser_stage.c
 * @brief State machine parser for clap
 */

#include "clap_parser_internal.h"

static bool is_action_value_required(clap_argument_t *arg) {
    return arg->nargs != 0 &&
           arg->action != CLAP_ACTION_STORE_TRUE &&
           arg->action != CLAP_ACTION_STORE_FALSE &&
           arg->action != CLAP_ACTION_STORE_CONST &&
           arg->action != CLAP_ACTION_APPEND_CONST &&
           arg->action != CLAP_ACTION_COUNT;
}

size_t clap_match_nargs(clap_argument_t *arg,
                        const char *pattern,
                        size_t start_idx,
                        size_t pattern_len) {
    if (!arg || !pattern || start_idx >= pattern_len) {
        return 0;
    }

    size_t count = 0;
    while (start_idx + count < pattern_len && pattern[start_idx + count] == PATTERN_ARGUMENT) {
        count++;
    }

    switch (arg->nargs) {
    case CLAP_NARGS_ZERO_OR_ONE:
        return count > 0 ? 1 : 0;
    case CLAP_NARGS_ZERO_OR_MORE:
        return count;
    case CLAP_NARGS_ONE_OR_MORE:
        return count;
    case CLAP_NARGS_REMAINDER:
        return pattern_len - start_idx;
    case CLAP_NARGS_PARSER:
        return count;
    default:
        if (arg->nargs > 0) {
            return count >= (size_t)arg->nargs ? (size_t)arg->nargs : count;
        }
        return count > 0 ? 1 : 0;
    }
}

bool clap_validate_nargs_count(clap_argument_t *arg,
                                size_t consumed,
                                clap_error_t *error) {
    if (!arg) {
        if (error) {
            clap_error_set(error, CLAP_ERR_INVALID_ARGUMENT, "Invalid argument for nargs validation");
        }
        return false;
    }

    switch (arg->nargs) {
    case CLAP_NARGS_DEFAULT:
        if (consumed != 1) {
            clap_error_set(error, CLAP_ERR_TOO_FEW_ARGS,
                           "argument %s: expected 1 argument, got %zu",
                           clap_buffer_cstr(arg->display_name), consumed);
            return false;
        }
        return true;
    case CLAP_NARGS_ZERO_OR_ONE:
        if (consumed > 1) {
            clap_error_set(error, CLAP_ERR_TOO_MANY_ARGS,
                           "argument %s: expected at most 1 argument, got %zu",
                           clap_buffer_cstr(arg->display_name), consumed);
            return false;
        }
        return true;
    case CLAP_NARGS_ZERO_OR_MORE:
        return true;
    case CLAP_NARGS_ONE_OR_MORE:
        if (consumed < 1) {
            clap_error_set(error, CLAP_ERR_TOO_FEW_ARGS,
                           "argument %s: expected at least 1 argument, got 0",
                           clap_buffer_cstr(arg->display_name));
            return false;
        }
        return true;
    case CLAP_NARGS_REMAINDER:
        return true;
    case CLAP_NARGS_PARSER:
        if (consumed < 1) {
            clap_error_set(error, CLAP_ERR_TOO_FEW_ARGS,
                           "argument %s: expected at least 1 argument, got 0",
                           clap_buffer_cstr(arg->display_name));
            return false;
        }
        return true;
    default:
        if (arg->nargs > 0 && consumed != (size_t)arg->nargs) {
            clap_error_set(error, CLAP_ERR_TOO_FEW_ARGS,
                           "argument %s: expected %d argument(s), got %zu",
                           clap_buffer_cstr(arg->display_name), arg->nargs, consumed);
            return false;
        }
        return true;
    }
}

static clap_parse_result_t parse_single_option(clap_parser_t *parser,
                                token_t *token,
                                clap_namespace_t *ns,
                                token_t *next_token,
                                size_t remaining,
                                bool *mutex_group_used,
                                size_t *consumed,
                                clap_error_t *error) {
    if (!token || !parser || !ns || !consumed) {
        if (error) {
            clap_error_set(error, CLAP_ERR_INVALID_ARGUMENT, "Invalid option parse parameters");
        }
        return CLAP_PARSE_ERROR;
    }

    bool ambiguous = false;
    bool is_long = token->type != TOKEN_SHORT_OPTION;
    clap_argument_t *arg = clap_find_option_best_match(parser, token->option_name, is_long, &ambiguous);
    if (ambiguous) {
        clap_error_set(error, CLAP_ERR_UNRECOGNIZED,
                       "Ambiguous option '%s'", token->raw ? token->raw : token->option_name);
        return CLAP_PARSE_ERROR;
    }

    if (!arg) {
        clap_error_set(error, CLAP_ERR_UNRECOGNIZED,
                       "Unrecognized option '%s'", token->raw ? token->raw : token->option_name);
        return CLAP_PARSE_ERROR;
    }

    if (!clap_mutex_check_conflict(parser, arg, mutex_group_used, token->raw, error)) {
        return CLAP_PARSE_ERROR;
    }

    if (arg->action == CLAP_ACTION_HELP) {
        clap_print_help(parser, stdout);
        return CLAP_PARSE_HELP;
    }
    if (arg->action == CLAP_ACTION_VERSION) {
        clap_print_version(parser, stdout);
        return CLAP_PARSE_VERSION;
    }

    const char *value = NULL;
    *consumed = 1;

    if (token->type == TOKEN_LONG_OPTION_EQ) {
        value = token->value;
    } else if (is_action_value_required(arg) && remaining > 1) {
        if (next_token == NULL) {
            clap_error_set(error, CLAP_ERR_INVALID_ARGUMENT,
                           "Missing required argument for option '%s'",
                           token->raw ? token->raw : token->option_name);
            return CLAP_PARSE_ERROR;
        }

        if (next_token->type == TOKEN_POSITIONAL) {
            value = next_token->raw;
            *consumed = 2;
        } else if (arg->nargs != CLAP_NARGS_ZERO_OR_ONE) {
            clap_error_set(error, CLAP_ERR_MISSING_VALUE,
                           "argument %s: expected a value, got '%s'",
                           token->raw ? token->raw : token->option_name,
                           next_token->raw ? next_token->raw : "");
            return CLAP_PARSE_ERROR;
        }
        /* nargs='?': next token is an option, leave value as NULL */
    }

    if (!clap_apply_argument_action(parser, arg, ns, value, error)) {
        return CLAP_PARSE_ERROR;
    }

    /* Handle fixed nargs > 1 for optional arguments */
    if (arg->nargs > 1 && arg->nargs != CLAP_NARGS_ZERO_OR_MORE &&
        arg->nargs != CLAP_NARGS_ONE_OR_MORE && arg->nargs != CLAP_NARGS_REMAINDER) {
        size_t nargs_needed = (size_t)arg->nargs;
        size_t values_collected = 1;

        if (next_token == NULL) {
            clap_error_set(error, CLAP_ERR_INVALID_ARGUMENT,
                           "argument '%s': token sequence ended unexpectedly",
                           token->raw ? token->raw : token->option_name);
            return CLAP_PARSE_ERROR;
        }

        for (size_t i = 1; i < nargs_needed; i++) {
            if (*consumed + 1 > remaining) {
                clap_error_set(error, CLAP_ERR_TOO_FEW_ARGS,
                               "argument '%s': expected %d argument(s), got %zu",
                               token->raw ? token->raw : token->option_name,
                               arg->nargs, values_collected);
                return CLAP_PARSE_ERROR;
            }

            token_t *val_token = &next_token[i];
            if (val_token->type == TOKEN_STOP) {
                clap_error_set(error, CLAP_ERR_TOO_FEW_ARGS,
                               "argument '%s': expected %d argument(s), got %zu",
                               token->raw ? token->raw : token->option_name,
                               arg->nargs, values_collected);
                return CLAP_PARSE_ERROR;
            }

            if (!clap_apply_argument_action(parser, arg, ns, val_token->raw, error)) {
                return CLAP_PARSE_ERROR;
            }

            (*consumed)++;
            values_collected++;
        }
    }

    if (arg->nargs == CLAP_NARGS_REMAINDER && remaining > *consumed) {
        if (next_token == NULL) {
            clap_error_set(error, CLAP_ERR_INVALID_ARGUMENT,
                           "Cannot process remainder arguments: next_token is NULL");
            return CLAP_PARSE_ERROR;
        }
        for (size_t i = *consumed; i < remaining; i++) {
            token_t *remaining_token = &next_token[i - 1];
            if (remaining_token->type == TOKEN_STOP) {
                continue;
            }
            if (!clap_namespace_append_string(ns, clap_buffer_cstr(arg->dest), remaining_token->raw)) {
                clap_error_set(error, CLAP_ERR_MEMORY, "Failed to store remainder values");
                return CLAP_PARSE_ERROR;
            }
        }
        *consumed = remaining;
    }

    return CLAP_PARSE_SUCCESS;
}

static clap_parse_result_t parse_short_bundle(clap_parser_t *parser,
                               token_t *token,
                               clap_token_t *tokens,
                               size_t token_count,
                               size_t pos,
                               clap_namespace_t *ns,
                               bool *mutex_group_used,
                               size_t *consumed,
                               clap_error_t *error) {
    size_t bundle_count = 0;
    char **expanded = clap_expand_short_bundle(token->option_name, &bundle_count);
    if (!expanded) {
        clap_error_set(error, CLAP_ERR_MEMORY, "Failed to expand option bundle");
        return CLAP_PARSE_ERROR;
    }

    size_t total_consumed = 1;
    for (size_t i = 0; i < bundle_count; i++) {
        token_t sub_token = clap_tokenize_arg(expanded[i]);
        size_t sub_consumed = 0;
        token_t *next_token = (pos + total_consumed < token_count) ? &tokens[pos + total_consumed] : NULL;
        size_t remaining = token_count - pos;

        clap_parse_result_t r = parse_single_option(parser, &sub_token, ns, next_token, remaining, mutex_group_used, &sub_consumed, error);
        if (r != CLAP_PARSE_SUCCESS) {
            for (size_t j = 0; j < bundle_count; j++) {
                clap_free(expanded[j]);
            }
            clap_free(expanded);
            return r;
        }

        if (sub_consumed > 1) {
            total_consumed += sub_consumed - 1;
        }
    }

    for (size_t i = 0; i < bundle_count; i++) {
        clap_free(expanded[i]);
    }
    clap_free(expanded);

    *consumed = total_consumed;
    return CLAP_PARSE_SUCCESS;
}

clap_parse_result_t clap_parse_with_pattern(clap_parser_t *parser,
                             clap_token_t *tokens,
                             clap_pattern_t *pattern,
                             clap_namespace_t *ns,
                             bool *mutex_group_used,
                             clap_error_t *error) {
    if (!parser || !pattern || !ns || (pattern->pattern_len > 0 && !tokens)) {
        if (error) {
            clap_error_set(error, CLAP_ERR_INVALID_ARGUMENT, "Invalid parser stage parameters");
        }
        return CLAP_PARSE_ERROR;
    }

    size_t positional_index = 0;
    size_t pos = 0;
    bool parsing_options = true;

    while (pos < pattern->pattern_len) {
        clap_token_t *token = &tokens[pos];

        if (token->type == TOKEN_STOP) {
            parsing_options = false;
            pos++;
            continue;
        }

        if (parsing_options && token->type == TOKEN_POSITIONAL) {
            parsing_options = false;
        }

        if (parsing_options) {
            if (token->type == TOKEN_SHORT_OPTION_BUNDLE) {
                size_t consumed = 0;
                clap_parse_result_t r = parse_short_bundle(parser, token, tokens,
                    pattern->pattern_len, pos, ns,
                    mutex_group_used, &consumed, error);
                if (r != CLAP_PARSE_SUCCESS) {
                    return r;
                }
                pos += consumed;
                continue;
            }

            size_t consumed = 0;
            clap_token_t *next = (pos + 1 < pattern->pattern_len) ? &tokens[pos + 1] : NULL;
            size_t remaining = pattern->pattern_len - pos;
            clap_parse_result_t r = parse_single_option(parser, token, ns, next,
                remaining, mutex_group_used, &consumed, error);
            if (r != CLAP_PARSE_SUCCESS) {
                return r;
            }
            pos += consumed;
            continue;
        }

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
                    if (strcmp(cmd_name, token->raw) == 0) {
                        subparser = sub;
                        break;
                    }
                }
            }

            if (subparser) {
                size_t remaining = pattern->pattern_len - pos;
                char **sub_argv = clap_calloc(remaining > 0 ? remaining : 1, sizeof(char*));
                if (!sub_argv) {
                    clap_error_set(error, CLAP_ERR_MEMORY, "Failed to allocate subcommand argv");
                    return CLAP_PARSE_ERROR;
                }

                /* Build argv for the subparser in the same shape as a normal main(). */
                sub_argv[0] = (char*)clap_buffer_cstr(subparser->prog_name);
                for (size_t i = 1; i < remaining; i++) {
                    sub_argv[i] = (char*)tokens[pos + i].raw;
                }

                clap_namespace_t *sub_ns = NULL;
                clap_parse_result_t sub_result = clap_parse_args(subparser,
                    (int)remaining, sub_argv, &sub_ns, error);
                if (sub_result != CLAP_PARSE_SUCCESS) {
                    error->subcommand_name = token->raw;
                    clap_free(sub_argv);
                    return sub_result;
                }

                clap_free(sub_argv);
                clap_namespace_set_string(ns, clap_buffer_cstr(parser->subparser_dest), token->raw);
                clap_namespace_merge(ns, sub_ns);
                clap_namespace_free(sub_ns);
                return CLAP_PARSE_SUCCESS;
            }
        }

        if (positional_index >= parser->positional_count) {
            clap_error_set(error, CLAP_ERR_TOO_MANY_ARGS,
                           "Unexpected argument '%s'", token->raw);
            return CLAP_PARSE_ERROR;
        }

        clap_argument_t *pos_arg = parser->positional_args[positional_index];
        int nargs_needed = pos_arg->nargs;

        if (pos_arg->choices && pos_arg->choice_count > 0) {
            if (!clap_validate_choice(pos_arg, token->raw, error)) {
                return CLAP_PARSE_ERROR;
            }
        }

        if (nargs_needed == CLAP_NARGS_REMAINDER) {
            for (size_t i = pos; i < pattern->pattern_len; i++) {
                clap_token_t *remaining = &tokens[i];
                if (remaining->type == TOKEN_STOP) {
                    continue;
                }
                if (!clap_namespace_append_string(ns, clap_buffer_cstr(pos_arg->dest), remaining->raw)) {
                    clap_error_set(error, CLAP_ERR_MEMORY, "Failed to store remainder values");
                    return CLAP_PARSE_ERROR;
                }
            }
            return CLAP_PARSE_SUCCESS;
        }

        if (nargs_needed > 1 && nargs_needed != CLAP_NARGS_ZERO_OR_MORE &&
            nargs_needed != CLAP_NARGS_ONE_OR_MORE && nargs_needed != CLAP_NARGS_ZERO_OR_ONE) {
            if (pos + (size_t)nargs_needed > pattern->pattern_len) {
                clap_error_set(error, CLAP_ERR_TOO_FEW_ARGS,
                               "argument %s: expected %d argument(s), got %zu",
                               clap_buffer_cstr(pos_arg->display_name), nargs_needed,
                               pattern->pattern_len - pos);
                return CLAP_PARSE_ERROR;
            }
            for (size_t i = 0; i < (size_t)nargs_needed; i++) {
                clap_token_t *value_token = &tokens[pos + i];
                if (!clap_namespace_append_string(ns, clap_buffer_cstr(pos_arg->dest), value_token->raw)) {
                    clap_error_set(error, CLAP_ERR_MEMORY, "Failed to store positional values");
                    return CLAP_PARSE_ERROR;
                }
            }
            pos += (size_t)nargs_needed;
            positional_index++;
            continue;
        }

        if (pos_arg->flags & CLAP_ARG_MULTIPLE) {
            if (!clap_namespace_append_string(ns, clap_buffer_cstr(pos_arg->dest), token->raw)) {
                clap_error_set(error, CLAP_ERR_MEMORY, "Failed to store positional values");
                return CLAP_PARSE_ERROR;
            }
            pos++;
            continue;
        }

        if (!clap_apply_argument_action(parser, pos_arg, ns, token->raw, error)) {
            return CLAP_PARSE_ERROR;
        }

        pos++;
        positional_index++;
    }

    return CLAP_PARSE_SUCCESS;
}
