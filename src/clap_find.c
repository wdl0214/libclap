/**
* @file clap_find.c
 * @brief Option lookup implementation
 */

#include "clap_parser_internal.h"

clap_argument_t* clap_find_option(clap_parser_t *parser, const char *name, bool is_long) {
    if (!parser || !name) return NULL;

    char search_key[256];
    if (is_long) {
        snprintf(search_key, sizeof(search_key), "--%s", name);
    } else {
        snprintf(search_key, sizeof(search_key), "-%s", name);
    }

    for (size_t i = 0; i < parser->optional_count; i++) {
        clap_argument_t *arg = parser->optional_args[i];
        for (size_t j = 0; j < arg->option_count; j++) {
            if (strcmp(arg->option_strings[j], search_key) == 0) {
                return arg;
            }
        }
    }

    return NULL;
}

clap_argument_t* clap_find_option_best_match(clap_parser_t *parser, const char *name, bool is_long, bool *ambiguous) {
    if (ambiguous) {
        *ambiguous = false;
    }
    if (!parser || !name) return NULL;

    char search_key[256];
    if (is_long) {
        snprintf(search_key, sizeof(search_key), "--%s", name);
    } else {
        snprintf(search_key, sizeof(search_key), "-%s", name);
    }

    /* Exact match first */
    for (size_t i = 0; i < parser->optional_count; i++) {
        clap_argument_t *arg = parser->optional_args[i];
        for (size_t j = 0; j < arg->option_count; j++) {
            if (strcmp(arg->option_strings[j], search_key) == 0) {
                return arg;
            }
        }
    }

    if (!is_long || !parser->allow_abbrev) {
        return NULL;
    }

    size_t name_len = strlen(name);
    clap_argument_t *match = NULL;
    bool seen_ambiguous = false;

    for (size_t i = 0; i < parser->optional_count; i++) {
        clap_argument_t *arg = parser->optional_args[i];
        for (size_t j = 0; j < arg->option_count; j++) {
            const char *opt = arg->option_strings[j];
            if (strncmp(opt, "--", 2) != 0) {
                continue;
            }
            const char *long_name = opt + 2;
            if (strncmp(long_name, name, name_len) != 0) {
                continue;
            }
            if (match && match != arg) {
                seen_ambiguous = true;
            } else if (!match) {
                match = arg;
            }
            break;
        }
    }

    if (ambiguous) {
        *ambiguous = seen_ambiguous;
    }
    return seen_ambiguous ? NULL : match;
}

clap_argument_t* clap_find_option_fast(clap_parser_t *parser, const char *name, bool is_long) {
    /* For now, just use the linear search */
    return clap_find_option(parser, name, is_long);
}
