/**
 * @file clap.h
 * @brief Main public API for libclap
 * 
 * libclap is a modern, secure, and extensible command-line argument
 * parsing library for C, inspired by Python's argparse.
 */

#ifndef CLAP_H
#define CLAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <clap/clap_version.h>
#include <clap/clap_error.h>
#include <clap/clap_types.h>
#include <clap/clap_action.h>
#include <clap/clap_allocator.h> 

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * nargs Specifiers
 * ============================================================================ */

#define CLAP_NARGS_ZERO_OR_ONE   (-3)  /**< '?' - 0 or 1 argument */
#define CLAP_NARGS_ZERO_OR_MORE  (-4)  /**< '*' - 0 or more arguments */
#define CLAP_NARGS_ONE_OR_MORE   (-5)  /**< '+' - 1 or more arguments */
#define CLAP_NARGS_OPTIONAL      CLAP_NARGS_ZERO_OR_ONE
#define CLAP_NARGS_REMAINDER     (-1)  /**< Consume all remaining arguments */
#define CLAP_NARGS_PARSER        (-2)  /**< Consume for subparser */

/* ============================================================================
 * Parser API
 * ============================================================================ */

clap_parser_t* clap_parser_new(
    const char *prog_name,
    const char *description,
    const char *epilog
);

void clap_parser_free(clap_parser_t *parser);

void clap_parser_set_help_width(clap_parser_t *parser, int width);
void clap_parser_set_allow_abbrev(clap_parser_t *parser, bool allow);
void clap_parser_set_version(clap_parser_t *parser, const char *version);

/* ============================================================================
 * Argument API
 * ============================================================================ */

clap_argument_t* clap_add_argument(
    clap_parser_t *parser,
    const char *name_or_flags
);

clap_argument_t* clap_argument_help(clap_argument_t *arg, const char *help_text);
clap_argument_t* clap_argument_type(clap_argument_t *arg, const char *type_name);
clap_argument_t* clap_argument_default(clap_argument_t *arg, const char *default_value);
clap_argument_t* clap_argument_required(clap_argument_t *arg, bool required);
clap_argument_t* clap_argument_choices(clap_argument_t *arg, const char **choices, size_t count);
clap_argument_t* clap_argument_nargs(clap_argument_t *arg, int nargs);
clap_argument_t* clap_argument_action(clap_argument_t *arg, clap_action_t action);
clap_argument_t* clap_argument_metavar(clap_argument_t *arg, const char *metavar);
clap_argument_t* clap_argument_dest(clap_argument_t *arg, const char *dest);
clap_argument_t* clap_argument_const(clap_argument_t *arg, const char *const_value);
clap_argument_t* clap_argument_handler(clap_argument_t *arg, clap_action_handler_t handler);
clap_argument_t* clap_argument_group(clap_argument_t *arg, int group_id);

/* ============================================================================
 * Mutually Exclusive Groups
 * ============================================================================ */

int clap_add_mutually_exclusive_group(clap_parser_t *parser, bool required);
bool clap_mutex_group_add_argument(clap_parser_t *parser, int group_id, clap_argument_t *arg);

/* ============================================================================
 * Subparsers
 * ============================================================================ */

clap_parser_t* clap_add_subparsers(
    clap_parser_t *parser,
    const char *dest,
    const char *help_text
);

clap_parser_t* clap_subparser_add(
    clap_parser_t *subparsers,
    const char *name,
    const char *help_text
);

void clap_subparsers_metavar(clap_parser_t *parser, const char *metavar);
bool clap_print_subcommand_help(clap_parser_t *parser, const char *command_name, FILE *stream);

/* ============================================================================
 * Parsing
 * ============================================================================ */

bool clap_parse_args(
    clap_parser_t *parser,
    int argc,
    char *argv[],
    clap_namespace_t **out_namespace,
    clap_error_t *error
);

/* ============================================================================
 * Namespace API
 * ============================================================================ */

void clap_namespace_free(clap_namespace_t *ns);

bool clap_namespace_get_string(clap_namespace_t *ns, const char *name, const char **value);
bool clap_namespace_get_int(clap_namespace_t *ns, const char *name, int *value);
bool clap_namespace_get_float(clap_namespace_t *ns, const char *name, double *value);
bool clap_namespace_get_bool(clap_namespace_t *ns, const char *name, bool *value);
bool clap_namespace_get_string_array(clap_namespace_t *ns, const char *name, 
                                      const char ***values, size_t *count);

bool clap_namespace_set_string(clap_namespace_t *ns, const char *name, const char *value);
bool clap_namespace_set_int(clap_namespace_t *ns, const char *name, int value);
bool clap_namespace_set_bool(clap_namespace_t *ns, const char *name, bool value);
bool clap_namespace_append_string(clap_namespace_t *ns, const char *name, const char *value);

/* ============================================================================
 * Help and Version
 * ============================================================================ */

void clap_print_help(clap_parser_t *parser, FILE *stream);
void clap_print_version(clap_parser_t *parser, FILE *stream);

/* ============================================================================
 * Extension APIs
 * ============================================================================ */

bool clap_register_type(
    clap_parser_t *parser,
    const char *type_name,
    clap_type_handler_t handler,
    size_t output_size
);

/* ============================================================================
 * Dependency API
 * ============================================================================ */

bool clap_argument_requires(
    clap_argument_t *arg,
    clap_argument_t *required_arg,
    const char *error_msg
);

bool clap_argument_conflicts(
    clap_argument_t *arg,
    clap_argument_t *conflicting_arg,
    const char *error_msg
);

#ifdef __cplusplus
}
#endif

#endif /* CLAP_H */