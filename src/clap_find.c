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

clap_argument_t* clap_find_option_fast(clap_parser_t *parser, const char *name, bool is_long) {
    /* For now, just use the linear search */
    return clap_find_option(parser, name, is_long);
}
