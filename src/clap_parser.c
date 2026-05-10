/**
 * @file clap_parser.c
 * @brief Parser implementation
 */

#include "clap_parser_internal.h"

clap_parser_t* clap_parser_new(const char *prog_name,
                                const char *description,
                                const char *epilog) {
    clap_parser_t *parser = clap_calloc(1, sizeof(clap_parser_t));
    if (!parser) return NULL;

    parser->prog_name = clap_buffer_new(prog_name ? prog_name : "program");
    if (description) parser->description = clap_buffer_new(description);
    if (epilog) parser->epilog = clap_buffer_new(epilog);

    parser->arg_capacity = 8;
    parser->arguments = clap_calloc(parser->arg_capacity, sizeof(clap_argument_t*));
    if (!parser->arguments) {
        clap_parser_free(parser);
        return NULL;
    }

    parser->help_width = 100;
    parser->add_help_option = true;
    parser->allow_abbrev = false;
    clap_color_theme_init(&parser->color_theme);
    parser->next_mutex_group_id = 0;
    parser->subparsers_container = NULL;

    clap_register_builtin_types(parser);

    /* Add default help option */
    if (parser->add_help_option) {
        clap_argument_t *help = clap_add_argument(parser, "--help/-h");
        if (help) {
            clap_argument_help(help, CLAP_TR("Show this help message and exit"));
            clap_argument_action(help, CLAP_ACTION_HELP);
        }
    }

    return parser;
}

void clap_parser_free(clap_parser_t *parser) {
    if (!parser) return;

    clap_buffer_free(parser->prog_name);
    clap_buffer_free(parser->description);
    clap_buffer_free(parser->epilog);
    clap_buffer_free(parser->version);
    clap_buffer_free(parser->subparser_dest);
    clap_buffer_free(parser->subparser_metavar);
    clap_buffer_free(parser->usage);

    /* Free arguments */
    for (size_t i = 0; i < parser->arg_count; i++) {
        clap_argument_free(parser->arguments[i]);
    }
    clap_free(parser->arguments);
    clap_free(parser->positional_args);
    clap_free(parser->optional_args);

    /* Free mutex groups */
    for (size_t i = 0; i < parser->mutex_group_count; i++) {
        clap_free(parser->mutex_groups[i]->arguments);
        clap_free(parser->mutex_groups[i]->title);
        clap_free(parser->mutex_groups[i]);
    }
    clap_free(parser->mutex_groups);

    /* Free display groups */
    for (size_t i = 0; i < parser->display_group_count; i++) {
        clap_free(parser->display_groups[i]->arguments);
        clap_free(parser->display_groups[i]->title);
        clap_free(parser->display_groups[i]->description);
        clap_free(parser->display_groups[i]);
    }
    clap_free(parser->display_groups);

    /* Free subparsers */
    for (size_t i = 0; i < parser->subparser_count; i++) {
        clap_parser_free(parser->subparsers[i]);
    }
    clap_free(parser->subparsers);

    /* Free subparsers container */
    if (parser->subparsers_container) {
        clap_parser_free(parser->subparsers_container);
    }

    /* Free type handlers */
    for (size_t i = 0; i < parser->type_handler_count; i++) {
        clap_free(parser->type_handlers[i].name);
    }
    clap_free(parser->type_handlers);

    clap_free(parser);
}

void clap_parser_set_help_width(clap_parser_t *parser, int width) {
    if (parser && width >= 40 && width <= 500) {
        parser->help_width = width;
    }
}

void clap_parser_set_allow_abbrev(clap_parser_t *parser, bool allow) {
    if (parser) {
        parser->allow_abbrev = allow;
    }
}

void clap_parser_set_version(clap_parser_t *parser, const char *version) {
    if (parser) {
        clap_buffer_free(parser->version);
        parser->version = version ? clap_buffer_new(version) : NULL;
    }
}

void clap_parser_set_color(clap_parser_t *parser, bool enable) {
    if (!parser) return;
    if (enable) {
        /* Auto-detect based on NO_COLOR, FORCE_COLOR, isatty, etc. */
        clap_color_theme_detect(&parser->color_theme, stdout);
    } else {
        parser->color_theme.enabled = false;
    }
}

void clap_parser_set_add_help(clap_parser_t *parser, bool add_help) {
    if (!parser) return;

    parser->add_help_option = add_help;

    /* Find existing help argument in parser->arguments */
    clap_argument_t *help_arg = NULL;
    size_t help_idx = (size_t)-1;
    for (size_t i = 0; i < parser->arg_count; i++) {
        if (parser->arguments[i]->action == CLAP_ACTION_HELP) {
            help_arg = parser->arguments[i];
            help_idx = i;
            break;
        }
    }

    if (add_help && !help_arg) {
        /* Add if not present */
        clap_argument_t *h = clap_add_argument(parser, "--help/-h");
        if (h) {
            clap_argument_help(h, CLAP_TR("Show this help message and exit"));
            clap_argument_action(h, CLAP_ACTION_HELP);
        }
    } else if (!add_help && help_arg) {
        /* Remove from parser->arguments */
        memmove(&parser->arguments[help_idx], &parser->arguments[help_idx + 1],
                (parser->arg_count - help_idx - 1) * sizeof(clap_argument_t*));
        parser->arg_count--;

        /* Remove from parser->optional_args */
        for (size_t i = 0; i < parser->optional_count; i++) {
            if (parser->optional_args[i] == help_arg) {
                memmove(&parser->optional_args[i], &parser->optional_args[i + 1],
                        (parser->optional_count - i - 1) * sizeof(clap_argument_t*));
                parser->optional_count--;
                break;
            }
        }

        /* Free the argument */
        clap_argument_free(help_arg);
    }
}

void clap_parser_set_usage(clap_parser_t *parser, const char *usage) {
    if (parser) {
        clap_buffer_free(parser->usage);
        parser->usage = (usage && *usage) ? clap_buffer_new(usage) : NULL;
    }
}


bool clap_register_type(clap_parser_t *parser,
                        const char *type_name,
                        clap_type_handler_t handler,
                        size_t output_size) {
    if (!parser || !type_name || !handler) return false;

    /* Check for duplicate */
    for (size_t i = 0; i < parser->type_handler_count; i++) {
        if (strcmp(parser->type_handlers[i].name, type_name) == 0) {
            return false;
        }
    }

    /* Expand if needed */
    if (parser->type_handler_count >= parser->type_handler_capacity) {
        size_t new_cap = parser->type_handler_capacity ?
                         parser->type_handler_capacity * 2 : 4;
        clap_type_entry_t *new_handlers = clap_realloc(
            parser->type_handlers, new_cap * sizeof(clap_type_entry_t)
        );
        if (!new_handlers) return false;
        parser->type_handlers = new_handlers;
        parser->type_handler_capacity = new_cap;
    }

    clap_type_entry_t *entry = &parser->type_handlers[parser->type_handler_count++];
    entry->name = clap_strdup(type_name);
    entry->handler = handler;
    entry->size = output_size;

    return true;
}
