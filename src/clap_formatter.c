/**
 * @file clap_formatter.c
 * @brief Help formatting implementation
 */

#include "clap_parser_internal.h"
#include <ctype.h>

static void wrap_text(clap_buffer_t **out, const char *text, int indent, int width, int first_line_already_indented) {
    if (!text || !*text) return;
    
    const char *p = text;
    int line_len = 0;
    int is_first_line = 1;
    
    while (*p) {
        while (*p && isspace((unsigned char)*p)) p++;
        if (!*p) break;
        
        const char *word_start = p;
        while (*p && !isspace((unsigned char)*p)) p++;
        size_t word_len = (size_t)(p - word_start);
        
        /* Calculate available width */
        int available;
        if (is_first_line && first_line_already_indented) {
            available = width;
        } else {
            available = width - indent;
        }
        if (available < 20) available = width - 20;
        
        if (line_len + (int)word_len + (line_len > 0 ? 1 : 0) > available) {
            clap_buffer_cat(out, "\n");
            for (int i = 0; i < indent; i++) {
                clap_buffer_cat(out, " ");
            }
            line_len = 0;
            is_first_line = 0;
        }
        
        /* Add word */
        if (is_first_line && first_line_already_indented) {
            if (line_len > 0) {
                clap_buffer_cat(out, " ");
                line_len++;
            }
            clap_buffer_cat_len(out, word_start, word_len);
            line_len += (int)word_len;
            is_first_line = 0;
        } else {
            if (line_len > 0) {
                clap_buffer_cat(out, " ");
                line_len++;
            }
            clap_buffer_cat_len(out, word_start, word_len);
            line_len += (int)word_len;
        }
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

/* Calculate the display length of an option string */
static size_t get_option_display_length(clap_argument_t *arg) {
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
    
    size_t len = clap_buffer_len(opt_str);
    clap_buffer_free(opt_str);
    return len;
}

void clap_print_help(clap_parser_t *parser, FILE *stream) {
    if (!parser || !stream) return;
    
    clap_buffer_t *buf = clap_buffer_empty();
    int width = parser->help_width;
    
    /* ========== Build usage line ========== */
    clap_buffer_cat_printf(&buf, "usage: %s", clap_buffer_cstr(parser->prog_name));
    
    /* Always add -h first */
    clap_buffer_cat(&buf, " [-h]");
    
    /* 1. Regular optional arguments (not in mutex groups) */
    for (size_t i = 0; i < parser->optional_count; i++) {
        clap_argument_t *arg = parser->optional_args[i];
        
        if (arg->action == CLAP_ACTION_HELP) continue;
        if (arg->group_id >= 0) continue;
        
        if (arg->action == CLAP_ACTION_VERSION) {
            clap_buffer_cat_printf(&buf, " [--version]");
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
            clap_buffer_cat_printf(&buf, " %s", clap_buffer_cstr(opt_str));
        } else {
            clap_buffer_cat_printf(&buf, " [%s]", clap_buffer_cstr(opt_str));
        }
        
        clap_buffer_free(opt_str);
    }
    
    /* 2. Mutually exclusive groups */
    for (size_t g = 0; g < parser->mutex_group_count; g++) {
        clap_mutex_group_t *group = parser->mutex_groups[g];
        
        if (group->arg_count == 0) continue;
        
        clap_buffer_t *group_str = clap_buffer_empty();
        for (size_t j = 0; j < group->arg_count; j++) {
            if (j > 0) clap_buffer_cat(&group_str, " | ");
            clap_argument_t *arg = group->arguments[j];
            
            const char *opt_name = arg->option_strings[0];
            clap_buffer_cat(&group_str, opt_name);
            
            if (arg->nargs != 0 && action_requires_value(arg->action)) {
                char upper_metavar[64];
                get_upper_metavar(arg, upper_metavar, sizeof(upper_metavar));
                clap_buffer_cat_printf(&group_str, " %s", upper_metavar);
            }
        }
        
        if (clap_buffer_len(group_str) > 0) {
            if (group->required) {
                clap_buffer_cat_printf(&buf, " (%s)", clap_buffer_cstr(group_str));
            } else {
                clap_buffer_cat_printf(&buf, " [%s]", clap_buffer_cstr(group_str));
            }
        }
        clap_buffer_free(group_str);
    }
    
    /* 3. Positional arguments */
    for (size_t i = 0; i < parser->positional_count; i++) {
        clap_argument_t *arg = parser->positional_args[i];
        const char *name = clap_buffer_cstr(arg->display_name);
        
        if (arg->nargs == CLAP_NARGS_ZERO_OR_MORE) {
            clap_buffer_cat_printf(&buf, " [%s ...]", name);
        } else if (arg->nargs == CLAP_NARGS_ONE_OR_MORE) {
            clap_buffer_cat_printf(&buf, " %s [%s ...]", name, name);
        } else if (arg->nargs == CLAP_NARGS_ZERO_OR_ONE) {
            clap_buffer_cat_printf(&buf, " [%s]", name);
        } else if (arg->nargs == CLAP_NARGS_REMAINDER) {
            clap_buffer_cat(&buf, " ...");
        } else {
            clap_buffer_cat_printf(&buf, " %s", name);
        }
    }
    
    /* 4. Subcommands */
    if (parser->has_subparsers && parser->subparsers_container) {
        clap_parser_t *container = parser->subparsers_container;
        if (container->subparser_count > 0) {
            if (parser->subparser_metavar) {
                clap_buffer_cat_printf(&buf, " %s ...", 
                    clap_buffer_cstr(parser->subparser_metavar));
            } else {
                clap_buffer_cat(&buf, " {");
                for (size_t i = 0; i < container->subparser_count; i++) {
                    if (i > 0) clap_buffer_cat(&buf, ",");
                    const char *full_name = clap_buffer_cstr(container->subparsers[i]->prog_name);
                    const char *cmd_name = strrchr(full_name, ' ');
                    clap_buffer_cat(&buf, cmd_name ? cmd_name + 1 : full_name);
                }
                clap_buffer_cat(&buf, "} ...");
            }
        }
    }
    
    clap_buffer_cat(&buf, "\n");
    
    /* ========== Description ========== */
    if (parser->description && clap_buffer_len(parser->description) > 0) {
        clap_buffer_cat(&buf, "\n");
        wrap_text(&buf, clap_buffer_cstr(parser->description), 0, width, 0);
        clap_buffer_cat(&buf, "\n");
    }
    
    /* ========== Calculate maximum width for alignment ========== */
    size_t max_name_len = 0;
    
    for (size_t i = 0; i < parser->positional_count; i++) {
        clap_argument_t *arg = parser->positional_args[i];
        size_t len = clap_buffer_len(arg->display_name);
        if (len > max_name_len) max_name_len = len;
    }
    
    for (size_t i = 0; i < parser->optional_count; i++) {
        size_t len = get_option_display_length(parser->optional_args[i]);
        if (len > max_name_len) max_name_len = len;
    }
    
    if (parser->has_subparsers && parser->subparsers_container) {
        clap_parser_t *container = parser->subparsers_container;
        for (size_t i = 0; i < container->subparser_count; i++) {
            clap_parser_t *sub = container->subparsers[i];
            const char *full_name = clap_buffer_cstr(sub->prog_name);
            const char *cmd_name = strrchr(full_name, ' ');
            size_t len = strlen(cmd_name ? cmd_name + 1 : full_name);
            if (len > max_name_len) max_name_len = len;
        }
    }
    
    /* Intelligent capping based on terminal width */
    int max_allowed = width / 2;
    if (max_allowed < 20) max_allowed = 20;
    if ((int)max_name_len > max_allowed) {
        max_name_len = (size_t)max_allowed;
    }
    if (max_name_len < 10) max_name_len = 10;
    
    /* ========== Positional arguments section ========== */
    if (parser->positional_count > 0) {
        clap_buffer_cat(&buf, "\nPositional arguments:\n");
        for (size_t i = 0; i < parser->positional_count; i++) {
            clap_argument_t *arg = parser->positional_args[i];
            const char *name = clap_buffer_cstr(arg->display_name);
            
            clap_buffer_cat_printf(&buf, "  %s", name);
            
            size_t name_len = strlen(name);
            size_t padding = (name_len < max_name_len) ? (max_name_len - name_len + 4) : 4;
            for (size_t j = 0; j < padding; j++) {
                clap_buffer_cat(&buf, " ");
            }
            
            clap_buffer_t *full_help = clap_buffer_empty();
            if (arg->help_text) {
                clap_buffer_cat(&full_help, clap_buffer_cstr(arg->help_text));
            }
            if (arg->choices && arg->choice_count > 0) {
                if (clap_buffer_len(full_help) > 0) {
                    clap_buffer_cat(&full_help, " ");
                }
                clap_buffer_cat(&full_help, "(choices: ");
                for (size_t j = 0; j < arg->choice_count; j++) {
                    if (j > 0) clap_buffer_cat(&full_help, ", ");
                    clap_buffer_cat(&full_help, arg->choices[j]);
                }
                clap_buffer_cat(&full_help, ")");
            }
            if (arg->default_string) {
                if (clap_buffer_len(full_help) > 0) {
                    clap_buffer_cat(&full_help, " ");
                }
                clap_buffer_cat_printf(&full_help, "(default: %s)", 
                                       clap_buffer_cstr(arg->default_string));
            }
            
            if (clap_buffer_len(full_help) > 0) {
                int help_indent = 2 + (int)name_len + (int)padding;
                if (help_indent > width - 20) {
                    clap_buffer_cat(&buf, "\n");
                    for (int j = 0; j < (int)max_name_len + 6; j++) {
                        clap_buffer_cat(&buf, " ");
                    }
                    wrap_text(&buf, clap_buffer_cstr(full_help), (int)max_name_len + 6, width, 0);
                } else {
                    wrap_text(&buf, clap_buffer_cstr(full_help), help_indent, width, 1);
                }
            }
            clap_buffer_cat(&buf, "\n");
            clap_buffer_free(full_help);
        }
    }
    
    /* ========== Optional arguments section ========== */
    if (parser->optional_count > 0) {
        clap_buffer_cat(&buf, "\nOptional arguments:\n");
        for (size_t i = 0; i < parser->optional_count; i++) {
            clap_argument_t *arg = parser->optional_args[i];
            
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
            
            clap_buffer_cat_printf(&buf, "  %s", clap_buffer_cstr(opt_str));
            
            size_t opt_len = clap_buffer_len(opt_str);
            size_t padding = (opt_len < max_name_len) ? (max_name_len - opt_len + 4) : 4;
            for (size_t j = 0; j < padding; j++) {
                clap_buffer_cat(&buf, " ");
            }
            
            clap_buffer_t *full_help = clap_buffer_empty();
            if (arg->help_text) {
                clap_buffer_cat(&full_help, clap_buffer_cstr(arg->help_text));
            }
            if (arg->default_string) {
                if (clap_buffer_len(full_help) > 0) {
                    clap_buffer_cat(&full_help, " ");
                }
                clap_buffer_cat_printf(&full_help, "(default: %s)", 
                                       clap_buffer_cstr(arg->default_string));
            }
            
            if (clap_buffer_len(full_help) > 0) {
                int help_indent = 2 + (int)opt_len + (int)padding;

                if (help_indent > width - 20) {
                    clap_buffer_cat(&buf, "\n");
                    for (int j = 0; j < (int)max_name_len + 6; j++) {
                        clap_buffer_cat(&buf, " ");
                    }
                    wrap_text(&buf, clap_buffer_cstr(full_help), (int)max_name_len + 6, width, 0);
                } else {
                    wrap_text(&buf, clap_buffer_cstr(full_help), help_indent, width, 1);
                }
            }
            clap_buffer_cat(&buf, "\n");
            
            clap_buffer_free(opt_str);
            clap_buffer_free(full_help);
        }
    }
    
    /* ========== Commands section ========== */
    if (parser->has_subparsers && parser->subparsers_container) {
        clap_parser_t *container = parser->subparsers_container;
        if (container->subparser_count > 0) {
            clap_buffer_cat(&buf, "\nCommands:\n");
            for (size_t i = 0; i < container->subparser_count; i++) {
                clap_parser_t *sub = container->subparsers[i];
                const char *full_name = clap_buffer_cstr(sub->prog_name);
                const char *cmd_name = strrchr(full_name, ' ');
                cmd_name = cmd_name ? cmd_name + 1 : full_name;
                const char *desc = sub->description ? 
                    clap_buffer_cstr(sub->description) : "";
                
                clap_buffer_cat_printf(&buf, "  %s", cmd_name);
                
                size_t name_len = strlen(cmd_name);
                size_t padding = (name_len < max_name_len) ? (max_name_len - name_len + 4) : 4;
                for (size_t j = 0; j < padding; j++) {
                    clap_buffer_cat(&buf, " ");
                }
                
                if (desc && *desc) {
                    int help_indent = 2 + (int)name_len + (int)padding;
                    if (help_indent > width - 20) {
                        clap_buffer_cat(&buf, "\n");
                        for (int j = 0; j < (int)max_name_len + 6; j++) {
                            clap_buffer_cat(&buf, " ");
                        }
                        wrap_text(&buf, desc, (int)max_name_len + 6, width, 0);
                    } else {
                        wrap_text(&buf, desc, help_indent, width, 1);
                    }
                }
                clap_buffer_cat(&buf, "\n");
            }
        }
    }
    
    /* ========== Epilog ========== */
    if (parser->epilog && clap_buffer_len(parser->epilog) > 0) {
        clap_buffer_cat(&buf, "\n");
        wrap_text(&buf, clap_buffer_cstr(parser->epilog), 0, width, 0);
        clap_buffer_cat(&buf, "\n");
    }
    
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