/**
 * @file clap_formatter.c
 * @brief Help formatting implementation
 */

#include "clap_parser_internal.h"
#include <ctype.h>

/* Write n spaces into buf */
static void buffer_pad(clap_buffer_t **buf, int n) {
    for (int i = 0; i < n; i++) clap_buffer_cat(buf, " ");
}

/* Extract the bare command name from a prog_name like "app subcommand" */
static const char *subcommand_name(const char *full_name) {
    const char *p = strrchr(full_name, ' ');
    return p ? p + 1 : full_name;
}

static void wrap_text(clap_buffer_t **out, const char *text, int indent, int width, bool on_first_line) {
    if (!text || !*text) return;

    const char *p = text;
    int line_len = 0;

    while (*p) {
        while (*p && isspace((unsigned char)*p)) p++;
        if (!*p) break;

        const char *word_start = p;
        while (*p && !isspace((unsigned char)*p)) p++;
        size_t word_len = (size_t)(p - word_start);

        int available = on_first_line ? width : width - indent;
        if (available < 20) available = width - 20;

        if (line_len + (int)word_len + (line_len > 0 ? 1 : 0) > available) {
            clap_buffer_cat(out, "\n");
            buffer_pad(out, indent);
            line_len = 0;
            on_first_line = false;
        }

        if (line_len > 0) {
            clap_buffer_cat(out, " ");
            line_len++;
        }
        clap_buffer_cat_len(out, word_start, word_len);
        line_len += (int)word_len;
        on_first_line = false;
    }
}

/* Helper to get uppercase metavar */
static void get_upper_metavar(clap_argument_t *arg, char *out, size_t size) {
    /* If choices exist, use {choice1,choice2} format - keep original case */
    if (arg->choices && arg->choice_count > 0) {
        size_t pos = 0;
        out[pos++] = '{';
        for (size_t i = 0; i < arg->choice_count && pos < size - 2; i++) {
            if (i > 0) out[pos++] = ',';
            const char *choice = arg->choices[i];
            for (size_t j = 0; choice[j] && pos < size - 2; j++) {
                out[pos++] = choice[j];
            }
        }
        out[pos++] = '}';
        out[pos] = '\0';
        return;
    }
    
    /* No choices - use metavar or dest, uppercased */
    const char *metavar = arg->metavar ? 
        clap_buffer_cstr(arg->metavar) : clap_buffer_cstr(arg->dest);
    size_t j;
    for (j = 0; metavar[j] && j < size - 1; j++) {
        out[j] = (char)toupper((unsigned char)metavar[j]);
    }
    out[j] = '\0';
}

/* Helper to check if an action requires a value placeholder in usage */
static bool action_requires_value(clap_action_t action) {
    return action != CLAP_ACTION_STORE_TRUE &&
           action != CLAP_ACTION_STORE_FALSE &&
           action != CLAP_ACTION_STORE_CONST &&
           action != CLAP_ACTION_APPEND_CONST &&
           action != CLAP_ACTION_COUNT &&
           action != CLAP_ACTION_HELP &&
           action != CLAP_ACTION_VERSION;
}

/* Build the display string for an optional argument (e.g. "-f, --foo VALUE") */
static clap_buffer_t *build_opt_str(clap_argument_t *arg) {
    clap_buffer_t *opt_str = clap_buffer_empty();
    for (size_t j = 0; j < arg->option_count; j++) {
        if (j > 0) clap_buffer_cat(&opt_str, ", ");
        clap_buffer_cat(&opt_str, arg->option_strings[j]);
    }
    if (arg->metavar) {
        clap_buffer_cat_printf(&opt_str, " %s", clap_buffer_cstr(arg->metavar));
    } else if (arg->choices && arg->choice_count > 0) {
        clap_buffer_cat(&opt_str, " {");
        for (size_t j = 0; j < arg->choice_count; j++) {
            if (j > 0) clap_buffer_cat(&opt_str, ",");
            clap_buffer_cat(&opt_str, arg->choices[j]);
        }
        clap_buffer_cat(&opt_str, "}");
    } else if (arg->nargs > 0 && action_requires_value(arg->action)) {
        clap_buffer_cat_printf(&opt_str, " %s", clap_buffer_cstr(arg->dest));
    }
    return opt_str;
}

/* Build the full help text for an argument (help + choices + default) */
static clap_buffer_t *build_arg_help(clap_argument_t *arg) {
    clap_buffer_t *help = clap_buffer_empty();
    if (arg->help_text) {
        clap_buffer_cat(&help, clap_buffer_cstr(arg->help_text));
    }
    if (arg->choices && arg->choice_count > 0) {
        if (clap_buffer_len(help) > 0) clap_buffer_cat(&help, " ");
        clap_buffer_cat(&help, "(choices: ");
        for (size_t j = 0; j < arg->choice_count; j++) {
            if (j > 0) clap_buffer_cat(&help, ", ");
            clap_buffer_cat(&help, arg->choices[j]);
        }
        clap_buffer_cat(&help, ")");
    }
    if (arg->default_string) {
        if (clap_buffer_len(help) > 0) clap_buffer_cat(&help, " ");
        clap_buffer_cat_printf(&help, "(default: %s)", clap_buffer_cstr(arg->default_string));
    }
    return help;
}

/* Emit help text with wrapping; handles the case where help_indent is too wide */
static void emit_help_text(clap_buffer_t **buf, const char *text, int help_indent,
                           int fallback_indent, int width, bool already_indented) {
    if (help_indent > width - 20) {
        clap_buffer_cat(buf, "\n");
        buffer_pad(buf, fallback_indent);
        wrap_text(buf, text, fallback_indent, width, false);
    } else {
        wrap_text(buf, text, help_indent, width, already_indented);
    }
}

/* Output a blank-line-wrapped paragraph (used for description and epilog) */
static void print_paragraph(clap_buffer_t **buf, clap_buffer_t *text, int width) {
    if (!text || clap_buffer_len(text) == 0) return;
    clap_buffer_cat(buf, "\n");
    wrap_text(buf, clap_buffer_cstr(text), 0, width, false);
    clap_buffer_cat(buf, "\n");
}

/* Calculate the display length of an option string without allocating */
static size_t get_option_display_length(clap_argument_t *arg) {
    size_t len = 0;
    for (size_t j = 0; j < arg->option_count; j++) {
        if (j > 0) len += 2; /* ", " */
        len += strlen(arg->option_strings[j]);
    }
    if (arg->metavar) {
        len += 1 + clap_buffer_len(arg->metavar);
    } else if (arg->choices && arg->choice_count > 0) {
        len += 2; /* " {" */
        for (size_t j = 0; j < arg->choice_count; j++) {
            if (j > 0) len += 1;
            len += strlen(arg->choices[j]);
        }
        len += 1; /* "}" */
    } else if (arg->nargs > 0 && action_requires_value(arg->action)) {
        len += 1 + clap_buffer_len(arg->dest);
    }
    return len;
}

/* Append a single item row "  NAME<padding>HELP" to buf */
static void append_help_row(clap_buffer_t **buf, const char *name, size_t name_len,
                            const char *help_text, size_t max_name_len, int width) {
    clap_buffer_cat_printf(buf, "  %s", name);
    size_t padding = (name_len < max_name_len) ? (max_name_len - name_len + 4) : 4;
    buffer_pad(buf, (int)padding);
    if (help_text && *help_text) {
        int help_indent = 2 + (int)name_len + (int)padding;
        emit_help_text(buf, help_text, help_indent, (int)max_name_len + 6, width, true);
    }
    clap_buffer_cat(buf, "\n");
}

/* Build the usage line into buf */
static void build_usage_line(clap_parser_t *parser, clap_buffer_t **buf) {
    clap_buffer_cat_printf(buf, "usage: %s", clap_buffer_cstr(parser->prog_name));
    clap_buffer_cat(buf, " [-h]");

    /* Regular optional arguments (not in mutex groups) */
    for (size_t i = 0; i < parser->optional_count; i++) {
        clap_argument_t *arg = parser->optional_args[i];
        if (arg->action == CLAP_ACTION_HELP) continue;
        if (arg->mutex_group_id != CLAP_MUTEX_GROUP_NONE) continue;

        if (arg->action == CLAP_ACTION_VERSION) {
            clap_buffer_cat(buf, " [--version]");
            continue;
        }

        clap_buffer_t *opt_str = clap_buffer_empty();
        clap_buffer_cat(&opt_str, arg->option_strings[0]);
        if (arg->nargs != 0 && action_requires_value(arg->action)) {
            char upper_metavar[64];
            get_upper_metavar(arg, upper_metavar, sizeof(upper_metavar));
            if (arg->nargs == CLAP_NARGS_ZERO_OR_ONE && arg->default_string) {
                clap_buffer_cat_printf(&opt_str, " [%s]", upper_metavar);
            } else {
                clap_buffer_cat_printf(&opt_str, " %s", upper_metavar);
            }
        }
        if (arg->flags & CLAP_ARG_REQUIRED) {
            clap_buffer_cat_printf(buf, " %s", clap_buffer_cstr(opt_str));
        } else {
            clap_buffer_cat_printf(buf, " [%s]", clap_buffer_cstr(opt_str));
        }
        clap_buffer_free(opt_str);
    }

    /* Mutually exclusive groups */
    for (size_t g = 0; g < parser->mutex_group_count; g++) {
        clap_mutex_group_t *group = parser->mutex_groups[g];
        if (group->arg_count == 0) continue;

        clap_buffer_t *group_str = clap_buffer_empty();
        for (size_t j = 0; j < group->arg_count; j++) {
            if (j > 0) clap_buffer_cat(&group_str, " | ");
            clap_argument_t *arg = group->arguments[j];
            clap_buffer_cat(&group_str, arg->option_strings[0]);
            if (arg->nargs != 0 && action_requires_value(arg->action)) {
                char upper_metavar[64];
                get_upper_metavar(arg, upper_metavar, sizeof(upper_metavar));
                clap_buffer_cat_printf(&group_str, " %s", upper_metavar);
            }
        }
        if (clap_buffer_len(group_str) > 0) {
            if (group->required) {
                clap_buffer_cat_printf(buf, " (%s)", clap_buffer_cstr(group_str));
            } else {
                clap_buffer_cat_printf(buf, " [%s]", clap_buffer_cstr(group_str));
            }
        }
        clap_buffer_free(group_str);
    }

    /* Positional arguments */
    for (size_t i = 0; i < parser->positional_count; i++) {
        clap_argument_t *arg = parser->positional_args[i];
        const char *name = clap_buffer_cstr(arg->display_name);
        if (arg->nargs == CLAP_NARGS_ZERO_OR_MORE) {
            clap_buffer_cat_printf(buf, " [%s ...]", name);
        } else if (arg->nargs == CLAP_NARGS_ONE_OR_MORE) {
            clap_buffer_cat_printf(buf, " %s [%s ...]", name, name);
        } else if (arg->nargs == CLAP_NARGS_ZERO_OR_ONE) {
            clap_buffer_cat_printf(buf, " [%s]", name);
        } else if (arg->nargs == CLAP_NARGS_REMAINDER) {
            clap_buffer_cat(buf, " ...");
        } else {
            clap_buffer_cat_printf(buf, " %s", name);
        }
    }

    /* Subcommands */
    if (parser->has_subparsers && parser->subparsers_container) {
        clap_parser_t *container = parser->subparsers_container;
        if (container->subparser_count > 0) {
            if (parser->subparser_metavar) {
                clap_buffer_cat_printf(buf, " %s ...",
                    clap_buffer_cstr(parser->subparser_metavar));
            } else {
                clap_buffer_cat(buf, " {");
                for (size_t i = 0; i < container->subparser_count; i++) {
                    if (i > 0) clap_buffer_cat(buf, ",");
                    clap_buffer_cat(buf, subcommand_name(
                        clap_buffer_cstr(container->subparsers[i]->prog_name)));
                }
                clap_buffer_cat(buf, "} ...");
            }
        }
    }

    clap_buffer_cat(buf, "\n");
}

/* Compute the column width used to align help text across all sections */
static size_t calc_max_name_len(clap_parser_t *parser, int width) {
    size_t max_name_len = 0;

    for (size_t i = 0; i < parser->positional_count; i++) {
        size_t len = clap_buffer_len(parser->positional_args[i]->display_name);
        if (len > max_name_len) max_name_len = len;
    }
    for (size_t i = 0; i < parser->optional_count; i++) {
        size_t len = get_option_display_length(parser->optional_args[i]);
        if (len > max_name_len) max_name_len = len;
    }
    if (parser->has_subparsers && parser->subparsers_container) {
        clap_parser_t *container = parser->subparsers_container;
        for (size_t i = 0; i < container->subparser_count; i++) {
            size_t len = strlen(subcommand_name(
                clap_buffer_cstr(container->subparsers[i]->prog_name)));
            if (len > max_name_len) max_name_len = len;
        }
    }

    int max_allowed = width / 2;
    if (max_allowed < 20) max_allowed = 20;
    if ((int)max_name_len > max_allowed) max_name_len = (size_t)max_allowed;
    if (max_name_len < 10) max_name_len = 10;
    return max_name_len;
}

static void print_positionals_section(clap_parser_t *parser, clap_buffer_t **buf,
                                      size_t max_name_len, int width) {
    if (parser->positional_count == 0) return;
    clap_buffer_cat(buf, "\nPositional arguments:\n");
    for (size_t i = 0; i < parser->positional_count; i++) {
        clap_argument_t *arg = parser->positional_args[i];
        if (arg->display_group_id != CLAP_DISPLAY_GROUP_NONE) continue;
        const char *name = clap_buffer_cstr(arg->display_name);
        clap_buffer_t *full_help = build_arg_help(arg);
        append_help_row(buf, name, strlen(name),
                        clap_buffer_cstr(full_help), max_name_len, width);
        clap_buffer_free(full_help);
    }
}

static void print_optionals_section(clap_parser_t *parser, clap_buffer_t **buf,
                                    size_t max_name_len, int width) {
    bool has_content = false;
    for (size_t i = 0; i < parser->optional_count; i++) {
        if (parser->optional_args[i]->display_group_id == CLAP_DISPLAY_GROUP_NONE) {
            has_content = true;
            break;
        }
    }
    if (!has_content) return;

    clap_buffer_cat(buf, "\nOptional arguments:\n");
    for (size_t i = 0; i < parser->optional_count; i++) {
        clap_argument_t *arg = parser->optional_args[i];
        if (arg->display_group_id != CLAP_DISPLAY_GROUP_NONE) continue;
        clap_buffer_t *opt_str = build_opt_str(arg);
        clap_buffer_t *full_help = build_arg_help(arg);
        append_help_row(buf, clap_buffer_cstr(opt_str), clap_buffer_len(opt_str),
                        clap_buffer_cstr(full_help), max_name_len, width);
        clap_buffer_free(opt_str);
        clap_buffer_free(full_help);
    }
}

static void print_display_groups_sections(clap_parser_t *parser, clap_buffer_t **buf,
                                           size_t max_name_len, int width) {
    if (parser->display_group_count == 0) return;

    for (size_t g = 0; g < parser->display_group_count; g++) {
        clap_display_group_t *group = parser->display_groups[g];
        if (group->arg_count == 0) continue;

        clap_buffer_cat_printf(buf, "\n%s:\n", group->title);

        if (group->description) {
            clap_buffer_cat(buf, "  ");
            wrap_text(buf, group->description, 2, width, true);
            clap_buffer_cat(buf, "\n");
        }

        for (size_t i = 0; i < group->arg_count; i++) {
            clap_argument_t *arg = group->arguments[i];
            if (arg->flags & CLAP_ARG_POSITIONAL) {
                const char *name = clap_buffer_cstr(arg->display_name);
                clap_buffer_t *full_help = build_arg_help(arg);
                append_help_row(buf, name, strlen(name),
                                clap_buffer_cstr(full_help), max_name_len, width);
                clap_buffer_free(full_help);
            } else {
                clap_buffer_t *opt_str = build_opt_str(arg);
                clap_buffer_t *full_help = build_arg_help(arg);
                append_help_row(buf, clap_buffer_cstr(opt_str), clap_buffer_len(opt_str),
                                clap_buffer_cstr(full_help), max_name_len, width);
                clap_buffer_free(opt_str);
                clap_buffer_free(full_help);
            }
        }
    }
}

static void print_commands_section(clap_parser_t *parser, clap_buffer_t **buf,
                                   size_t max_name_len, int width) {
    if (!parser->has_subparsers || !parser->subparsers_container) return;
    clap_parser_t *container = parser->subparsers_container;
    if (container->subparser_count == 0) return;

    clap_buffer_cat(buf, "\nCommands:\n");
    for (size_t i = 0; i < container->subparser_count; i++) {
        clap_parser_t *sub = container->subparsers[i];
        const char *cmd_name = subcommand_name(clap_buffer_cstr(sub->prog_name));
        const char *desc = sub->description ? clap_buffer_cstr(sub->description) : "";
        append_help_row(buf, cmd_name, strlen(cmd_name), desc, max_name_len, width);
    }
}

void clap_print_help(clap_parser_t *parser, FILE *stream) {
    if (!parser || !stream) return;

    clap_buffer_t *buf = clap_buffer_empty();
    int width = parser->help_width;

    build_usage_line(parser, &buf);
    print_paragraph(&buf, parser->description, width);
    size_t max_name_len = calc_max_name_len(parser, width);
    print_positionals_section(parser, &buf, max_name_len, width);
    print_optionals_section(parser, &buf, max_name_len, width);
    print_display_groups_sections(parser, &buf, max_name_len, width);
    print_commands_section(parser, &buf, max_name_len, width);
    print_paragraph(&buf, parser->epilog, width);

    fprintf(stream, "%s", clap_buffer_cstr(buf));
    clap_buffer_free(buf);
}

void clap_print_version(clap_parser_t *parser, FILE *stream) {
    if (!parser || !stream) return;
    
    const char *version = parser->version ? 
        clap_buffer_cstr(parser->version) : "unknown";
    
    fprintf(stream, "%s version %s\n", 
            clap_buffer_cstr(parser->prog_name), version);
}
