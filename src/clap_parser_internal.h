/**
 * @file clap_parser_internal.h
 * @brief Internal parser structures
 * @internal
 */

#ifndef CLAP_PARSER_INTERNAL_H
#define CLAP_PARSER_INTERNAL_H

#include <clap/clap.h>

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>

#ifdef _MSC_VER
#pragma warning(disable: 4200)  // nonstandard extension: zero-sized array
#endif

/* Maximum values */
#define CLAP_MAX_ARGUMENTS      256
#define CLAP_MAX_ARG_LENGTH     4096
#define CLAP_MAX_CHOICES        1000
#define CLAP_MAX_DEPENDENCIES   16

/* Group identifier constants */
#define CLAP_MUTEX_GROUP_NONE    (-1)
#define CLAP_DISPLAY_GROUP_NONE  (-1)

#ifdef _WIN32
#include <float.h>
#define clap_isfinite(x) _finite(x)
#define clap_isnan(x) _isnan(x)
#define clap_isinf(x) (!_finite(x) && !_isnan(x))
#else
#include <math.h>
#define clap_isfinite(x) isfinite(x)
#define clap_isnan(x) isnan(x)
#define clap_isinf(x) isinf(x)
#endif

/* Argument flags */
typedef enum {
    CLAP_ARG_POSITIONAL = 1u << 0,
    CLAP_ARG_OPTIONAL   = 1u << 1,
    CLAP_ARG_REQUIRED   = 1u << 2,
    CLAP_ARG_MULTIPLE   = 1u << 3,
    CLAP_ARG_CONSUMED   = 1u << 4,
} clap_arg_flags_t;

static inline void clap_flags_set(clap_arg_flags_t *flags, clap_arg_flags_t flag) {
    *flags |= flag;
}

static inline void clap_flags_clear(clap_arg_flags_t *flags, clap_arg_flags_t flag) {
    *flags &= ~((unsigned int)flag);
}

static inline bool clap_flags_has(clap_arg_flags_t flags, clap_arg_flags_t flag) {
    return (flags & flag) != 0;
}

/* ============================================================================
 * Internal Forward Declarations
 * ============================================================================ */

typedef struct clap_buffer_s clap_buffer_t;
typedef struct clap_arena_s clap_arena_t;
typedef struct clap_trie_s clap_trie_t;
typedef struct clap_mutex_group_s clap_mutex_group_t;
typedef struct clap_display_group_s clap_display_group_t;
typedef struct clap_dependency_s clap_dependency_t;

/* ============================================================================
 * Argument Structure
 * ============================================================================ */

struct clap_argument_s {
    /* Identity */
    clap_buffer_t *display_name;
    char **option_strings;
    size_t option_count;
    clap_buffer_t *dest;

    /* Configuration */
    clap_action_t action;
    clap_action_handler_t action_handler;
    void *action_data;

    /* Add const value for STORE_CONST and APPEND_CONST */
    clap_buffer_t *const_value;

    clap_type_handler_t type_handler;
    size_t type_size;
    clap_buffer_t *type_name;

    int nargs;
    clap_buffer_t *help_text;
    clap_buffer_t *metavar;

    /* Validation */
    char **choices;
    size_t choice_count;
    void *default_value;
    clap_buffer_t *default_string;

    /* State */
    clap_arg_flags_t flags;
    int mutex_group_id;
    int display_group_id;
    int position;

    /* Dependencies */
    clap_dependency_t **dependencies;
    size_t dependency_count;

    /* Subparser */
    clap_parser_t *subparser_ref;
};

/* ============================================================================
 * Mutex Group Structure
 * ============================================================================ */

struct clap_mutex_group_s {
    int id;
    bool required;
    char *title;
    clap_argument_t **arguments;
    size_t arg_count;
    size_t arg_capacity;

    struct {
        clap_argument_t *selected;
        int parse_count;
        const char *first_occurrence;
    } state;
};

/* ============================================================================
 * Display Group Structure
 * ============================================================================ */

struct clap_display_group_s {
    int id;
    char *title;
    char *description;
    clap_argument_t **arguments;
    size_t arg_count;
    size_t arg_capacity;
};

/* ============================================================================
 * Dependency Types and Structure
 * ============================================================================ */

typedef enum {
    CLAP_DEP_REQUIRES,
    CLAP_DEP_CONFLICTS,
    CLAP_DEP_REQUIRES_ANY,
    CLAP_DEP_REQUIRES_ALL,
    CLAP_DEP_IMPLIES,
} clap_dependency_type_t;

struct clap_dependency_s {
    clap_dependency_type_t type;
    clap_argument_t *source;
    clap_argument_t **targets;
    size_t target_count;
    char *error_message;
};

/* ============================================================================
 * Type Registry
 * ============================================================================ */

typedef struct {
    char *name;
    clap_type_handler_t handler;
    size_t size;
} clap_type_entry_t;

/* ============================================================================
 * Parser Structure
 * ============================================================================ */

struct clap_parser_s {
    /* Metadata */
    clap_buffer_t *prog_name;
    clap_buffer_t *description;
    clap_buffer_t *epilog;
    clap_buffer_t *version;

    /* Arguments */
    clap_argument_t **arguments;
    size_t arg_count;
    size_t arg_capacity;

    clap_argument_t **positional_args;
    size_t positional_count;
    clap_argument_t **optional_args;
    size_t optional_count;

    /* Groups */
    clap_mutex_group_t **mutex_groups;
    size_t mutex_group_count;
    int next_mutex_group_id;

    clap_display_group_t **display_groups;
    size_t display_group_count;
    int next_display_group_id;

    /* Subparsers */
    clap_parser_t **subparsers;
    size_t subparser_count;
    clap_buffer_t *subparser_dest;
    bool has_subparsers;
    clap_parser_t *subparsers_container; 
    clap_buffer_t *subparser_metavar;

    /* Types */
    clap_type_entry_t *type_handlers;
    size_t type_handler_count;
    size_t type_handler_capacity;

    /* Options */
    int help_width;
    bool add_help_option;
    bool allow_abbrev;

    /* Error */
    clap_error_t last_error;
};

/* ============================================================================
 * Tokenizer Types
 * ============================================================================ */

typedef enum {
    TOKEN_END,
    TOKEN_POSITIONAL,
    TOKEN_LONG_OPTION,
    TOKEN_LONG_OPTION_EQ,
    TOKEN_SHORT_OPTION,
    TOKEN_SHORT_OPTION_BUNDLE,
    TOKEN_STOP,
} token_type_t;

typedef struct {
    token_type_t type;
    const char *raw;
    const char *option_name;
    const char *value;
    bool name_allocated;
} token_t;

typedef token_t clap_token_t;

/* ============================================================================
 * Pattern Types
 * ============================================================================ */

typedef enum {
    PATTERN_OPTION = 'O',
    PATTERN_ARGUMENT = 'A',
    PATTERN_STOP = '-'
} clap_pattern_char_t;

typedef struct {
    char *pattern;
    size_t pattern_len;
    size_t *option_indices;
    clap_argument_t ***option_matches;
    size_t *option_match_counts;
    size_t option_count;
} clap_pattern_t;

typedef enum {
    PARSE_STATE_START,
    PARSE_STATE_OPTIONS,
    PARSE_STATE_POSITIONALS,
    PARSE_STATE_REMAINDER,
    PARSE_STATE_DONE
} clap_parse_state_t;

/* ============================================================================
 * Internal Allocator API
 *
 * These wrap the configurable allocator set via clap_set_allocator().
 * All internal allocations must use these, never raw malloc/free.
 * ============================================================================ */

void* clap_malloc(size_t size);
void* clap_calloc(size_t nmemb, size_t size);
void clap_free(void *ptr);
void* clap_realloc(void *ptr, size_t size);
char* clap_strdup(const char *str);
char* clap_strndup(const char *str, size_t n);

/* ============================================================================
 * Internal Function Declarations
 * ============================================================================ */

clap_token_t clap_tokenize_arg(const char *arg);
clap_token_t* clap_tokenize(int argc, char *argv[], size_t *count, clap_error_t *error);
void clap_tokenize_free(clap_token_t *tokens, size_t count);
char** clap_expand_short_bundle(const char *bundle, size_t *count);
bool check_required_positional(clap_argument_t *arg, clap_namespace_t *ns, clap_error_t *error);
bool check_required_option(clap_argument_t *arg, clap_namespace_t *ns, clap_error_t *error);
bool check_positional_nargs_constraint(clap_argument_t *arg, clap_namespace_t *ns, clap_error_t *error);

clap_pattern_t* clap_analyze_pattern(clap_parser_t *parser,
                                     clap_token_t *tokens,
                                     size_t token_count,
                                     clap_error_t *error);
void clap_pattern_free(clap_pattern_t *pattern);


size_t clap_match_nargs(clap_argument_t *arg,
                        const char *pattern,
                        size_t start_idx,
                        size_t pattern_len);
bool clap_validate_nargs_count(clap_argument_t *arg,
                                size_t consumed,
                                clap_error_t *error);

clap_parse_result_t clap_parse_with_pattern(clap_parser_t *parser,
                             clap_token_t *tokens,
                             clap_pattern_t *pattern,
                             clap_namespace_t *ns,
                             bool *mutex_group_used,
                             clap_error_t *error);

/* ============================================================================
 * Namespace Value Types
 * ============================================================================ */

typedef enum {
    CLAP_VAL_NONE,
    CLAP_VAL_STRING,
    CLAP_VAL_INT,
    CLAP_VAL_FLOAT,
    CLAP_VAL_BOOL,
    CLAP_VAL_ARRAY,
} clap_value_type_t;

/* ============================================================================
 * Namespace Value Structure
 * ============================================================================ */

typedef struct clap_value_s {
    char *name;
    clap_value_type_t type;
    union {
        char *str_val;
        int int_val;
        double float_val;
        bool bool_val;
        struct {
            char **items;
            size_t count;
            size_t capacity;
        } array;
        clap_namespace_t *ns_val;
    } data;
} clap_value_t;

/* ============================================================================
 * Namespace Structure
 * ============================================================================ */

struct clap_namespace_s {
    clap_value_t **values;
    size_t value_count;
    size_t value_capacity;
};

/* Action handlers */
bool clap_action_store(clap_parser_t *parser, clap_argument_t *arg,
                       clap_namespace_t *ns, const char **values,
                       size_t count, void *user_data, clap_error_t *error);
bool clap_action_store_const(clap_parser_t *parser, clap_argument_t *arg,
                              clap_namespace_t *ns, const char **values,
                              size_t count, void *user_data, clap_error_t *error);
bool clap_action_store_true(clap_parser_t *parser, clap_argument_t *arg,
                            clap_namespace_t *ns, const char **values,
                            size_t count, void *user_data, clap_error_t *error);
bool clap_action_store_false(clap_parser_t *parser, clap_argument_t *arg,
                             clap_namespace_t *ns, const char **values,
                             size_t count, void *user_data, clap_error_t *error);
bool clap_action_append(clap_parser_t *parser, clap_argument_t *arg,
                        clap_namespace_t *ns, const char **values,
                        size_t count, void *user_data, clap_error_t *error);
bool clap_action_append_const(clap_parser_t *parser, clap_argument_t *arg,
                               clap_namespace_t *ns, const char **values,
                               size_t count, void *user_data, clap_error_t *error);
bool clap_action_count(clap_parser_t *parser, clap_argument_t *arg,
                       clap_namespace_t *ns, const char **values,
                       size_t count, void *user_data, clap_error_t *error);
bool clap_action_custom(clap_parser_t *parser, clap_argument_t *arg,
                         clap_namespace_t *ns, const char **values,
                         size_t count, void *user_data, clap_error_t *error);
clap_action_handler_t get_action_handler(clap_action_t action);

bool clap_apply_argument_action(clap_parser_t *parser,
                                clap_argument_t *arg,
                                clap_namespace_t *ns,
                                const char *value,
                                clap_error_t *error);
/* ============================================================================
 * Buffer Structure (Internal)
 * ============================================================================ */

struct clap_buffer_s {
    size_t len;
    size_t alloc;
    char data[];
};

/* ============================================================================
 * Arena Structure (Internal)
 * ============================================================================ */

typedef struct clap_arena_chunk_s {
    struct clap_arena_chunk_s *next;
    size_t capacity;
    size_t used;
    uintptr_t data[];
} clap_arena_chunk_t;

struct clap_arena_s {
    clap_arena_chunk_t *current;
    clap_arena_chunk_t *first;
    size_t chunk_size;
};

/* ============================================================================
 * Trie Structure (Internal)
 * ============================================================================ */

typedef struct clap_trie_node_s {
    char character;
    clap_argument_t *argument;
    struct clap_trie_node_s *children[256];
    bool is_end_of_word;
} clap_trie_node_t;

struct clap_trie_s {
    clap_trie_node_t *root;
    clap_arena_t *arena;
    size_t node_count;
};

/* ============================================================================
 * Internal Function Declarations
 * ============================================================================ */

/* Buffer API */
clap_buffer_t* clap_buffer_new(const char *init);
clap_buffer_t* clap_buffer_new_len(const void *init, size_t len);
clap_buffer_t* clap_buffer_empty(void);
void clap_buffer_free(clap_buffer_t *buf);
bool clap_buffer_cat(clap_buffer_t **buf, const char *str);
bool clap_buffer_cat_len(clap_buffer_t **buf, const void *data, size_t len);
bool clap_buffer_cat_printf(clap_buffer_t **buf, const char *fmt, ...);
bool clap_buffer_copy(clap_buffer_t **buf, const char *str);
const char* clap_buffer_cstr(const clap_buffer_t *buf);
size_t clap_buffer_len(const clap_buffer_t *buf);
void clap_buffer_truncate(clap_buffer_t *buf, size_t len);
void clap_buffer_sanitize(clap_buffer_t *buf);

/* Arena API */
clap_arena_t* clap_arena_new(size_t chunk_size);
void clap_arena_free(clap_arena_t *arena);
void* clap_arena_alloc(clap_arena_t *arena, size_t size);
char* clap_arena_strdup(clap_arena_t *arena, const char *str);
void clap_arena_reset(clap_arena_t *arena);

/* Trie API */
clap_trie_t* clap_trie_new(void);
void clap_trie_free(clap_trie_t *trie);
bool clap_trie_insert(clap_trie_t *trie, const char *key, clap_argument_t *arg);
clap_argument_t* clap_trie_find_exact(clap_trie_t *trie, const char *key);
clap_argument_t* clap_trie_find_prefix(clap_trie_t *trie, const char *prefix, bool allow_ambiguous);

/* Validation */
bool clap_argument_validate(clap_argument_t *arg, clap_error_t *error);
bool clap_validate_mutex_groups(clap_parser_t *parser, clap_error_t *error);
bool clap_validate_dependencies(clap_parser_t *parser, clap_namespace_t *ns, clap_error_t *error);
bool clap_validate_choice(clap_argument_t *arg, const char *value, clap_error_t *error);
bool clap_validate_nargs(clap_argument_t *arg, size_t value_count, clap_error_t *error);

/* Defaults */
bool clap_apply_defaults(clap_parser_t *parser, clap_namespace_t *ns, clap_error_t *error);

/* Option lookup */
clap_argument_t* clap_find_option(clap_parser_t *parser, const char *name, bool is_long);
clap_argument_t* clap_find_option_fast(clap_parser_t *parser, const char *name, bool is_long);
clap_argument_t* clap_find_option_best_match(clap_parser_t *parser, const char *name, bool is_long, bool *ambiguous);

/* Namespace internal */
clap_namespace_t* clap_namespace_new(void);
bool clap_namespace_merge(clap_namespace_t *dst, clap_namespace_t *src);

/* Mutually Exclusive internal */
bool clap_mutex_check_conflict(clap_parser_t *parser,
                          clap_argument_t *arg,
                          bool *mutex_group_used,
                          const char *option_str,
                          clap_error_t *error);

#endif /* CLAP_PARSER_INTERNAL_H */
