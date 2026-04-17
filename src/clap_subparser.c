/**
 * @file clap_subparser.c
 * @brief Subparser implementation
 */

#include "clap_parser_internal.h"

clap_parser_t* clap_add_subparsers(clap_parser_t *parser,
                                    const char *dest,
                                    const char *help_text) {
    if (!parser || !dest) return NULL;
    
    parser->has_subparsers = true;
    parser->subparser_dest = clap_buffer_new(dest);
    
    /* Create a container parser for subcommands */
    clap_parser_t *subparsers = clap_parser_new("", help_text, "");
    if (!subparsers) return NULL;
    
    /* Inherit help width from parent */
    subparsers->help_width = parser->help_width;
    
    /* Inherit program name from parent */
    clap_buffer_free(subparsers->prog_name);
    subparsers->prog_name = clap_buffer_new(clap_buffer_cstr(parser->prog_name));
    
    /* Save container reference to main parser */
    parser->subparsers_container = subparsers;
    
    return subparsers;
}

clap_parser_t* clap_subparser_add(clap_parser_t *subparsers,
                                   const char *name,
                                   const char *help_text) {
    if (!subparsers || !name) return NULL;
    
    clap_parser_t *cmd_parser = clap_parser_new(name, help_text, "");
    if (!cmd_parser) return NULL;
    
    /* Inherit help width from parent */
    cmd_parser->help_width = subparsers->help_width;
    
    /* Build full program name: parent + " " + name */
    if (subparsers->prog_name && clap_buffer_len(subparsers->prog_name) > 0) {
        clap_buffer_t *full_name = clap_buffer_empty();
        clap_buffer_cat_printf(&full_name, "%s %s", 
                               clap_buffer_cstr(subparsers->prog_name), name);
        clap_buffer_free(cmd_parser->prog_name);
        cmd_parser->prog_name = full_name;
    }
    
    /* Add to subparsers list */
    clap_parser_t **new_subs = clap_realloc(
        subparsers->subparsers,
        (subparsers->subparser_count + 1) * sizeof(clap_parser_t*)
    );
    
    if (!new_subs) {
        clap_parser_free(cmd_parser);
        return NULL;
    }
    
    subparsers->subparsers = new_subs;
    subparsers->subparsers[subparsers->subparser_count++] = cmd_parser;
    
    return cmd_parser;
}

void clap_subparsers_metavar(clap_parser_t *parser, const char *metavar) {
    if (parser && metavar) {
        clap_buffer_free(parser->subparser_metavar);
        parser->subparser_metavar = clap_buffer_new(metavar);
    }
}

bool clap_print_subcommand_help(clap_parser_t *parser, const char *command_name, FILE *stream) {
    if (!parser || !command_name || !stream) return false;
    
    if (!parser->has_subparsers || !parser->subparsers_container) {
        return false;
    }
    
    clap_parser_t *container = parser->subparsers_container;
    
    for (size_t i = 0; i < container->subparser_count; i++) {
        clap_parser_t *sub = container->subparsers[i];
        
        const char *full_name = clap_buffer_cstr(sub->prog_name);
        const char *cmd_name = strrchr(full_name, ' ');
        if (cmd_name) {
            cmd_name++;
        } else {
            cmd_name = full_name;
        }
        
        if (strcmp(cmd_name, command_name) == 0) {
            clap_print_help(sub, stream);
            return true;
        }
    }
    
    return false;
}
