/**
 * @file clap_pattern.c
 * @brief Pattern analysis for tokenized command line arguments
 */

#include "clap_parser_internal.h"


static void free_pattern_partial(clap_pattern_t *pattern, size_t option_count) {
    if (!pattern) return;
    if (pattern->pattern) {
        clap_free(pattern->pattern);
    }
    if (pattern->option_indices) {
        clap_free(pattern->option_indices);
    }
    if (pattern->option_matches) {
        for (size_t i = 0; i < option_count; i++) {
            if (pattern->option_matches[i]) {
                clap_free(pattern->option_matches[i]);
            }
        }
        clap_free(pattern->option_matches);
    }
    if (pattern->option_match_counts) {
        clap_free(pattern->option_match_counts);
    }
    clap_free(pattern);
}

clap_pattern_t* clap_analyze_pattern(clap_parser_t *parser,
                                     clap_token_t *tokens,
                                     size_t token_count,
                                     clap_error_t *error) {
    if (!parser || (token_count > 0 && !tokens)) {
        if (error) {
            clap_error_set(error, CLAP_ERR_INVALID_ARGUMENT, "Invalid pattern analysis parameters");
        }
        return NULL;
    }

    clap_pattern_t *pattern = clap_calloc(1, sizeof(clap_pattern_t));
    if (!pattern) {
        clap_error_set(error, CLAP_ERR_MEMORY, "Failed to allocate pattern");
        return NULL;
    }

    pattern->pattern = clap_malloc(token_count + 1);
    if (token_count > 0) {
        pattern->option_indices = clap_calloc(token_count, sizeof(size_t));
        pattern->option_matches = clap_calloc(token_count, sizeof(clap_argument_t**));
        pattern->option_match_counts = clap_calloc(token_count, sizeof(size_t));
    }

    if (!pattern->pattern ||
        (token_count > 0 && (!pattern->option_indices || !pattern->option_matches || !pattern->option_match_counts))) {
        free_pattern_partial(pattern, 0);
        clap_error_set(error, CLAP_ERR_MEMORY, "Failed to allocate pattern buffers");
        return NULL;
    }

    bool saw_stop = false;
    bool parsing_options = true;
    size_t option_index = 0;

    for (size_t i = 0; i < token_count; i++) {
        clap_token_t *token = &tokens[i];

        if (saw_stop) {
            pattern->pattern[i] = PATTERN_ARGUMENT;
            continue;
        }

        if (!parsing_options) {
            pattern->pattern[i] = PATTERN_ARGUMENT;
            continue;
        }

        switch (token->type) {
        case TOKEN_STOP:
            pattern->pattern[i] = PATTERN_STOP;
            saw_stop = true;
            parsing_options = false;
            break;

        case TOKEN_POSITIONAL:
            pattern->pattern[i] = PATTERN_ARGUMENT;
            parsing_options = false;
            break;

        case TOKEN_SHORT_OPTION_BUNDLE:
            pattern->pattern[i] = PATTERN_OPTION;
            pattern->option_indices[option_index] = i;
            pattern->option_match_counts[option_index] = 0;
            pattern->option_matches[option_index] = NULL;
            option_index++;
            break;

        case TOKEN_SHORT_OPTION:
        case TOKEN_LONG_OPTION:
        case TOKEN_LONG_OPTION_EQ: {
            pattern->pattern[i] = PATTERN_OPTION;
            pattern->option_indices[option_index] = i;
            bool ambiguous = false;
            bool is_long = (token->type != TOKEN_SHORT_OPTION);
            clap_argument_t *match = clap_find_option_best_match(parser, token->option_name, is_long, &ambiguous);

            if (ambiguous) {
                clap_error_set(error, CLAP_ERR_UNRECOGNIZED,
                               "Ambiguous option '%s'", token->raw ? token->raw : token->option_name);
                free_pattern_partial(pattern, option_index);
                return NULL;
            }

            if (!match) {
                clap_error_set(error, CLAP_ERR_UNRECOGNIZED,
                               "Unrecognized option '%s'", token->raw ? token->raw : token->option_name);
                free_pattern_partial(pattern, option_index);
                return NULL;
            }

            pattern->option_matches[option_index] = clap_calloc(1, sizeof(clap_argument_t*));
            if (!pattern->option_matches[option_index]) {
                free_pattern_partial(pattern, option_index);
                clap_error_set(error, CLAP_ERR_MEMORY, "Failed to allocate option match storage");
                return NULL;
            }
            pattern->option_matches[option_index][0] = match;
            pattern->option_match_counts[option_index] = 1;
            option_index++;
            break;
        }

        default:
            pattern->pattern[i] = PATTERN_ARGUMENT;
            break;
        }
    }

    pattern->pattern[token_count] = '\0';
    pattern->pattern_len = token_count;
    pattern->option_count = option_index;
    return pattern;
}

void clap_pattern_free(clap_pattern_t *pattern) {
    if (!pattern) return;

    if (pattern->pattern) {
        clap_free(pattern->pattern);
    }
    if (pattern->option_indices) {
        clap_free(pattern->option_indices);
    }
    if (pattern->option_matches) {
        for (size_t i = 0; i < pattern->option_count; i++) {
            if (pattern->option_matches[i]) {
                clap_free(pattern->option_matches[i]);
            }
        }
        clap_free(pattern->option_matches);
    }
    if (pattern->option_match_counts) {
        clap_free(pattern->option_match_counts);
    }

    clap_free(pattern);
}
