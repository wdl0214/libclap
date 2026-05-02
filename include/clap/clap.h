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
#include <clap/clap_export.h>

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
#define CLAP_NARGS_DEFAULT       (1)
/* ============================================================================
 * Parser API
 * ============================================================================ */

/**
 * @brief Create a new argument parser.
 * @param prog_name   Program name shown in usage/help (e.g. "git").
 * @param description Text shown after the usage line in help output.
 *                    May be NULL.
 * @param epilog      Text shown after all argument sections in help.
 *                    May be NULL.
 * @return New parser, or NULL on allocation failure.
 *         Free with clap_parser_free().
 */
CLAP_EXPORT clap_parser_t* clap_parser_new(
    const char *prog_name,
    const char *description,
    const char *epilog
);

/**
 * @brief Destroy a parser and all resources it owns.
 *
 * Frees every argument, group, subparser, type registry entry, and
 * internal buffer associated with @p parser.  Safely handles NULL.
 *
 * @param parser  Parser to free, or NULL (no-op).
 */
CLAP_EXPORT void clap_parser_free(clap_parser_t *parser);

/**
 * @brief Set help output width (default: 100).
 * @param parser  Target parser.
 * @param width   Width in characters.  Clamped to [40, 500].
 */
CLAP_EXPORT void clap_parser_set_help_width(clap_parser_t *parser, int width);
/**
 * @brief Enable or disable abbreviated option matching.
 *
 * When enabled, "--ver" will match "--verbose" if no other option
 * shares the prefix.  Abbrev matching uses a prefix trie and detects
 * ambiguity.  Default: disabled.
 *
 * @param parser Target parser.
 * @param allow  true to enable prefix abbreviation matching.
 */
CLAP_EXPORT void clap_parser_set_allow_abbrev(clap_parser_t *parser, bool allow);
/**
 * @brief Set the version string printed by clap_print_version().
 * @param parser  Target parser.
 * @param version Version string to display.  May be NULL to clear.
 */
CLAP_EXPORT void clap_parser_set_version(clap_parser_t *parser, const char *version);

/* ============================================================================
 * Argument API
 * ============================================================================ */

/**
 * @brief Register a new argument (positional or optional).
 *
 * For positional args, pass a bare name:  @c "filename"
 * For optional args, use @c "--long/-s" syntax (split on '/'):
 *   - "--output/-o"  creates both --output and -o
 *   - "--verbose"    creates only --verbose
 *
 * The destination key (dest) is auto-derived from the longest flag:
 * leading dashes stripped, internal dashes replaced with underscores.
 * Override with clap_argument_dest().
 *
 * @param parser         Target parser.
 * @param name_or_flags  Argument spec (see description).
 * @return The new argument on success, NULL on failure (allocation or
 *         invalid spec).  Configure via clap_argument_* setters.
 */
CLAP_EXPORT clap_argument_t* clap_add_argument(
    clap_parser_t *parser,
    const char *name_or_flags
);

/**
 * @brief Set the help text for an argument.
 * @param arg       Target argument.
 * @param help_text Help string shown in --help output.  May be NULL
 *                  to clear.
 * @return @p arg for chaining.
 */
CLAP_EXPORT clap_argument_t* clap_argument_help(clap_argument_t *arg, const char *help_text);
/**
 * @brief Set the type name for an argument.
 *
 * Must be one of the built-in types ("string", "int", "float", "bool")
 * or a name previously registered with clap_register_type().  Default:
 * "string".
 *
 * @param arg       Target argument.
 * @param type_name Type name string (not copied — kept as-is).
 * @return @p arg for chaining.
 */
CLAP_EXPORT clap_argument_t* clap_argument_type(clap_argument_t *arg, const char *type_name);
/**
 * @brief Set a default value for an argument.
 *
 * The value is stored as a string and converted to the argument's type
 * when clap_apply_defaults() runs during clap_parse_args().
 *
 * @param arg           Target argument.
 * @param default_value Default value string, or NULL to clear.
 * @return @p arg for chaining.
 */
CLAP_EXPORT clap_argument_t* clap_argument_default(clap_argument_t *arg, const char *default_value);
/**
 * @brief Mark an optional argument as required or not.
 *
 * Positional arguments ignore this call — they are always required
 * unless their nargs is '?', '*', or REMAINDER.
 *
 * @param arg      Target argument.
 * @param required true to require, false to make optional.
 * @return @p arg for chaining.
 */
CLAP_EXPORT clap_argument_t* clap_argument_required(clap_argument_t *arg, bool required);
/**
 * @brief Restrict an argument to a fixed set of choices.
 * @param arg     Target argument.
 * @param choices Array of NUL-terminated choice strings.  The array
 *                content is copied internally.
 * @param count   Number of elements in @p choices.
 * @return @p arg for chaining.
 */
CLAP_EXPORT clap_argument_t* clap_argument_choices(clap_argument_t *arg, const char **choices, size_t count);
/**
 * @brief Set the number of values consumed by this argument.
 *
 * Use one of the CLAP_NARGS_* constants or a positive integer N.
 * Character literals '?', '+', '*' are also accepted.
 *
 * @param arg   Target argument.
 * @param nargs Number of values.  CLAP_NARGS_DEFAULT (1) by default.
 * @return @p arg for chaining.
 */
CLAP_EXPORT clap_argument_t* clap_argument_nargs(clap_argument_t *arg, int nargs);
/**
 * @brief Set the action type for an argument.
 * @param arg    Target argument.
 * @param action Action type from clap_action_t.  Default: STORE.
 * @return @p arg for chaining.
 */
CLAP_EXPORT clap_argument_t* clap_argument_action(clap_argument_t *arg, clap_action_t action);
/**
 * @brief Override the metavar shown in help/usage for this argument.
 * @param arg     Target argument.
 * @param metavar Metavar string (e.g. "FILE").  May be NULL to reset.
 * @return @p arg for chaining.
 */
CLAP_EXPORT clap_argument_t* clap_argument_metavar(clap_argument_t *arg, const char *metavar);
/**
 * @brief Override the destination key (dest) in the namespace.
 *
 * By default dest is derived from the longest flag: dashes stripped
 * and internal dashes replaced with underscores.
 *
 * @param arg  Target argument.
 * @param dest Destination key under which parsed values are stored.
 * @return @p arg for chaining.
 */
CLAP_EXPORT clap_argument_t* clap_argument_dest(clap_argument_t *arg, const char *dest);
/**
 * @brief Set the constant value for STORE_CONST / APPEND_CONST actions.
 * @param arg         Target argument.
 * @param const_value Constant string stored when this argument appears.
 * @return @p arg for chaining.
 */
CLAP_EXPORT clap_argument_t* clap_argument_const(clap_argument_t *arg, const char *const_value);
/**
 * @brief Register a custom action handler for CLAP_ACTION_CUSTOM.
 * @param arg     Target argument with action=CLAP_ACTION_CUSTOM.
 * @param handler Function pointer invoked when the argument is parsed.
 * @return @p arg for chaining.
 */
CLAP_EXPORT clap_argument_t* clap_argument_handler(clap_argument_t *arg, clap_action_handler_t handler);
CLAP_EXPORT CLAP_DEPRECATED("Use clap_mutex_group_add_argument() instead")
clap_argument_t* clap_argument_mutex_group(clap_argument_t *arg, int mutex_group_id);

/* ============================================================================
 * Mutually Exclusive Groups
 * ============================================================================ */

/**
 * @brief Create a mutually exclusive group.
 *
 * Arguments added to this group via clap_mutex_group_add_argument()
 * cannot appear together on the command line.
 *
 * @param parser   Target parser.
 * @param required If true, exactly one argument from the group must
 *                 be provided.
 * @return Group ID (positive integer) on success, -1 on failure.
 *         Pass the ID to clap_mutex_group_add_argument().
 */
CLAP_EXPORT int clap_add_mutually_exclusive_group(clap_parser_t *parser, bool required);
/**
 * @brief Add an argument to a mutually exclusive group.
 *
 * This also sets the argument's mutex group, so there is no need to
 * call clap_argument_mutex_group() separately.
 *
 * @param parser         Target parser.
 * @param mutex_group_id Group ID returned by
 *                       clap_add_mutually_exclusive_group().
 * @param arg            Argument to add to the group.
 * @return true on success, false if @p mutex_group_id is invalid.
 */
CLAP_EXPORT bool clap_mutex_group_add_argument(clap_parser_t *parser, int mutex_group_id, clap_argument_t *arg);

/* ============================================================================
 * Argument Groups
 * ============================================================================ */

/**
 * @brief Create a display group to organize arguments in help output.
 *
 * Arguments in a display group are excluded from the default
 * "Optional arguments" / "Positional arguments" sections and shown
 * under a named group section instead.
 *
 * @param parser      Target parser.
 * @param title       Section title shown in help output.
 * @param description Optional description shown below the title.
 *                    May be NULL.
 * @return Group ID (positive integer) on success, -1 on failure.
 */
CLAP_EXPORT int clap_add_argument_group(clap_parser_t *parser, const char *title, const char *description);
/**
 * @brief Add an argument to a display group.
 * @param parser           Target parser.
 * @param display_group_id Group ID from clap_add_argument_group().
 * @param arg              Argument to assign to the group.
 * @return true on success, false if @p display_group_id is invalid.
 */
CLAP_EXPORT bool clap_argument_group_add_argument(clap_parser_t *parser, int display_group_id, clap_argument_t *arg);

/* ============================================================================
 * Subparsers
 * ============================================================================ */

/**
 * @brief Enable subcommand support.
 *
 * After calling this, use clap_subparser_add() to register individual
 * subcommands.  Each subcommand is itself a clap_parser_t that can
 * have its own arguments.
 *
 * @param parser   Target parser.
 * @param dest     Namespace key where the matched subcommand name
 *                 will be stored.
 * @param help_text Description shown in the "Commands:" section of
 *                 the parent help output.  May be NULL.
 * @return A container parser.  Pass this to clap_subparser_add().
 *         Returns NULL on failure.
 */
CLAP_EXPORT clap_parser_t* clap_add_subparsers(
    clap_parser_t *parser,
    const char *dest,
    const char *help_text
);

/**
 * @brief Register a subcommand.
 *
 * The new subparser inherits its help width and abbreviation setting
 * from the parent.  Add arguments to it with clap_add_argument().
 *
 * @param subparsers The container returned by clap_add_subparsers().
 * @param name       Subcommand name (e.g. "commit", "push").
 * @param help_text  Help text shown in the "Commands:" section.
 * @return A new parser for the subcommand, or NULL on failure.
 */
CLAP_EXPORT clap_parser_t* clap_subparser_add(
    clap_parser_t *subparsers,
    const char *name,
    const char *help_text
);

/**
 * @brief Override the metavar shown for subcommands in the usage line.
 * @param parser   Target parser (the parent, not the container).
 * @param metavar  Metavar string (e.g. "COMMAND").  If NULL, the
 *                 metavar defaults to "{cmd1,cmd2,...}".
 */
CLAP_EXPORT void clap_subparsers_metavar(clap_parser_t *parser, const char *metavar);
/**
 * @brief Print help for a specific subcommand to a stream.
 * @param parser        Target parser (the parent).
 * @param command_name  Subcommand name.
 * @param stream        Output stream (e.g. stdout, stderr).
 * @return true on success, false if @p command_name is not found.
 */
CLAP_EXPORT bool clap_print_subcommand_help(clap_parser_t *parser, const char *command_name, FILE *stream);

/* ============================================================================
 * Parsing
 * ============================================================================ */

/**
 * @brief Parse command-line arguments.
 *
 * Tokenizes @p argv, analyzes the token pattern, runs the state-
 * machine parser, validates all constraints (required args, mutex
 * groups, dependencies, nargs), and writes results into a new
 * namespace.
 *
 * @param parser        The configured parser.
 * @param argc          Argument count (from main()).
 * @param argv          Argument vector (from main()).  argv[0] is the
 *                      program name and is not parsed as an option.
 * @param out_namespace On success, receives a newly allocated namespace
 *                      containing all parsed values.  Free with
 *                      clap_namespace_free().  Not modified on error.
 * @param error         Set on CLAP_PARSE_ERROR with details.  Not
 *                      modified for HELP/VERSION/SUCCESS results.
 * @return CLAP_PARSE_SUCCESS on success, CLAP_PARSE_ERROR on failure,
 *         CLAP_PARSE_HELP if --help/-h was given (help already printed
 *         to stdout), CLAP_PARSE_VERSION similarly.
 */
CLAP_EXPORT clap_parse_result_t clap_parse_args(
    clap_parser_t *parser,
    int argc,
    char *argv[],
    clap_namespace_t **out_namespace,
    clap_error_t *error
);

/* ============================================================================
 * Namespace API
 * ============================================================================ */

/**
 * @brief Free a namespace and all values it contains.  NULL-safe.
 * @param ns Namespace to free, or NULL (no-op).
 */
CLAP_EXPORT void clap_namespace_free(clap_namespace_t *ns);

/**
 * @brief Retrieve a string value from the namespace.
 * @param ns    Target namespace.
 * @param name  Destination key.
 * @param value Receives a pointer to the internal string.  Do not free.
 * @return true if the key exists and is a string, false otherwise.
 */
CLAP_EXPORT bool clap_namespace_get_string(clap_namespace_t *ns, const char *name, const char **value);
/**
 * @brief Retrieve an int value from the namespace.
 * @param ns    Target namespace.
 * @param name  Destination key.
 * @param value Receives the integer value.
 * @return true if the key exists and is an int, false otherwise.
 */
CLAP_EXPORT bool clap_namespace_get_int(clap_namespace_t *ns, const char *name, int *value);
/**
 * @brief Retrieve a float value from the namespace.
 * @param ns    Target namespace.
 * @param name  Destination key.
 * @param value Receives the double value.
 * @return true if the key exists and is a float, false otherwise.
 */
CLAP_EXPORT bool clap_namespace_get_float(clap_namespace_t *ns, const char *name, double *value);
/**
 * @brief Retrieve a bool value from the namespace.
 * @param ns    Target namespace.
 * @param name  Destination key.
 * @param value Receives the bool value.
 * @return true if the key exists and is a bool, false otherwise.
 */
CLAP_EXPORT bool clap_namespace_get_bool(clap_namespace_t *ns, const char *name, bool *value);
/**
 * @brief Retrieve a string array from the namespace (APPEND action).
 * @param ns     Target namespace.
 * @param name   Destination key.
 * @param values Receives a pointer to the internal string array.
 *               Do not modify or free.
 * @param count  Receives the number of elements.
 * @return true if the key exists and is an array, false otherwise.
 */
CLAP_EXPORT bool clap_namespace_get_string_array(clap_namespace_t *ns, const char *name,
                                      const char ***values, size_t *count);

/**
 * @brief Set a string value in the namespace.
 * @param ns    Target namespace.
 * @param name  Destination key.
 * @param value NUL-terminated string to copy.
 * @return true on success, false on allocation failure.
 */
CLAP_EXPORT bool clap_namespace_set_string(clap_namespace_t *ns, const char *name, const char *value);
/**
 * @brief Set an int value in the namespace.
 * @param ns    Target namespace.
 * @param name  Destination key.
 * @param value Integer value to store.
 * @return true on success, false on allocation failure.
 */
CLAP_EXPORT bool clap_namespace_set_int(clap_namespace_t *ns, const char *name, int value);
/**
 * @brief Set a float value in the namespace.
 * @param ns    Target namespace.
 * @param name  Destination key.
 * @param value Double value to store.
 * @return true on success, false on allocation failure.
 */
CLAP_EXPORT bool clap_namespace_set_float(clap_namespace_t *ns, const char *name, double value);
/**
 * @brief Set a bool value in the namespace.
 * @param ns    Target namespace.
 * @param name  Destination key.
 * @param value Bool value to store.
 * @return true on success, false on allocation failure.
 */
CLAP_EXPORT bool clap_namespace_set_bool(clap_namespace_t *ns, const char *name, bool value);
/**
 * @brief Append a string to an array value in the namespace.
 * Creates the array if the key does not exist yet.
 * @param ns    Target namespace.
 * @param name  Destination key.
 * @param value String to append (copied internally).
 * @return true on success, false on allocation failure.
 */
CLAP_EXPORT bool clap_namespace_append_string(clap_namespace_t *ns, const char *name, const char *value);

/* ============================================================================
 * Help and Version
 * ============================================================================ */

/**
 * @brief Print the full help text (usage + all sections) to a stream.
 *
 * Composes the usage line, description, positional arguments, optional
 * arguments, display groups, subcommands, and epilog, with smart
 * word-wrapping at the configured width.
 *
 * @param parser Target parser.
 * @param stream Output stream (e.g. stdout, stderr).
 */
CLAP_EXPORT void clap_print_help(clap_parser_t *parser, FILE *stream);
/**
 * @brief Print "progname version X.Y.Z" to a stream.
 *
 * The version string is the one set by clap_parser_set_version().
 * Falls back to "unknown" if no version was set.
 *
 * @param parser Target parser.
 * @param stream Output stream.
 */
CLAP_EXPORT void clap_print_version(clap_parser_t *parser, FILE *stream);

/* ============================================================================
 * Extension APIs
 * ============================================================================ */

/**
 * @brief Register a custom type converter.
 *
 * After registration, arguments can use this type name with
 * clap_argument_type().  Custom type handlers are resolved at
 * parse time.  If a type name is used but never registered,
 * clap_parse_args() returns CLAP_PARSE_ERROR with code
 * CLAP_ERR_TYPE_CONVERSION and message "Unknown type".
 *
 * @param parser      Target parser.
 * @param type_name   Type name (e.g. "ip_addr", "regex").
 * @param handler     Conversion function: const char* → typed value.
 * @param output_size Size of the output buffer in bytes (e.g.
 *                    sizeof(my_type_t)).
 * @return true on success, false if @p type_name duplicates an
 *         existing entry or allocation fails.
 */
CLAP_EXPORT bool clap_register_type(
    clap_parser_t *parser,
    const char *type_name,
    clap_type_handler_t handler,
    size_t output_size
);

/* ============================================================================
 * Dependency API
 * ============================================================================ */

/**
 * @brief Declare that @p arg requires @p required_arg to also be present.
 *
 * If @p arg is provided but @p required_arg is not, parsing fails
 * with CLAP_ERR_DEPENDENCY_VIOLATION and @p error_msg.
 *
 * @param arg          Source argument.
 * @param required_arg Argument that must also be present.
 * @param error_msg    Error message shown on violation.  Copied
 *                     internally.
 * @return true on success, false on allocation failure.
 */
CLAP_EXPORT bool clap_argument_requires(
    clap_argument_t *arg,
    clap_argument_t *required_arg,
    const char *error_msg
);

/**
 * @brief Declare that @p arg and @p conflicting_arg cannot both appear.
 *
 * If both are provided, parsing fails with CLAP_ERR_DEPENDENCY_VIOLATION
 * and @p error_msg.
 *
 * @param arg             Source argument.
 * @param conflicting_arg Argument that conflicts with @p arg.
 * @param error_msg       Error message shown on violation.  Copied
 *                        internally.
 * @return true on success, false on allocation failure.
 */
CLAP_EXPORT bool clap_argument_conflicts(
    clap_argument_t *arg,
    clap_argument_t *conflicting_arg,
    const char *error_msg
);

#ifdef __cplusplus
}
#endif

#endif /* CLAP_H */
