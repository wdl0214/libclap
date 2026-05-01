/**
 * @file action_demo.c
 * @brief Demonstrate all available action types in libclap
 */

#include <clap/clap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ============================================================================
 * Custom Action Handler
 * ============================================================================ */

static bool custom_uppercase_handler(clap_parser_t *parser,
                                      clap_argument_t *arg,
                                      clap_namespace_t *ns,
                                      const char **values,
                                      size_t count,
                                      void *user_data,
                                      clap_error_t *error) {
    (void)parser;
    (void)arg;
    (void)user_data;
    (void)error;
    
    if (count == 0 || !values || !values[0]) return true;
    
    /* Convert value to uppercase before storing */
    const char *original = values[0];
    size_t len = strlen(original);
    char *upper = malloc(len + 1);
    if (!upper) return false;
    
    for (size_t i = 0; i < len; i++) {
        upper[i] = (char)toupper((unsigned char)original[i]);
    }
    upper[len] = '\0';
    
    /* Store in namespace with fixed key "to_upper" */
    bool result = clap_namespace_set_string(ns, "to_upper", upper);
    free(upper);
    return result;
}

/* ============================================================================
 * Main
 * ============================================================================ */

int main(int argc, char *argv[]) {
    clap_error_t error = {0};
    clap_namespace_t *ns = NULL;
    
    /* ========================================================================
     * Create parser
     * ======================================================================== */
    clap_parser_t *parser = clap_parser_new(
        "action_demo",
        "Demonstrates all action types in libclap",
        "Examples:\n"
        "  action_demo --name John --age 30 --verbose\n"
        "  action_demo --mode-fast --tags important --tags urgent\n"
        "  action_demo -vvv --add-item --add-item --to-upper hello"
    );
    
    if (!parser) {
        fprintf(stderr, "Failed to create parser\n");
        return EXIT_FAILURE;
    }
    
    clap_parser_set_help_width(parser, 100);
    clap_parser_set_version(parser, "1.0.0");
    
    /* ========================================================================
     * 1. STORE action (default)
     * ======================================================================== */
    clap_argument_t *name = clap_add_argument(parser, "--name");
    clap_argument_action(name, CLAP_ACTION_STORE);
    clap_argument_type(name, "string");
    clap_argument_help(name, "Your name (STORE action)");
    clap_argument_metavar(name, "NAME");
    
    clap_argument_t *age = clap_add_argument(parser, "--age");
    clap_argument_action(age, CLAP_ACTION_STORE);
    clap_argument_type(age, "int");
    clap_argument_help(age, "Your age (STORE action)");
    clap_argument_metavar(age, "AGE");
    
    /* ========================================================================
     * 2. STORE_TRUE / STORE_FALSE actions
     * ======================================================================== */

    /* Mutually exclusive group: quiet vs verbose */
    int verbosity_group = clap_add_mutually_exclusive_group(parser, false);
    
    clap_argument_t *verbose = clap_add_argument(parser, "--verbose/-v");
    clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);
    clap_argument_help(verbose, "Enable verbose output (STORE_TRUE action)");
    clap_mutex_group_add_argument(parser, verbosity_group, verbose);
    
    clap_argument_t *quiet = clap_add_argument(parser, "--quiet/-q");
    clap_argument_action(quiet, CLAP_ACTION_STORE_FALSE);
    clap_argument_dest(quiet, "quiet");
    clap_argument_help(quiet, "Disable verbose output (STORE_FALSE action)");
    clap_argument_mutex_group(quiet, verbosity_group);
    clap_mutex_group_add_argument(parser, verbosity_group, quiet);
    
    /* ========================================================================
     * 3. COUNT action
     * ======================================================================== */
    clap_argument_t *verbosity = clap_add_argument(parser, "-d");
    clap_argument_action(verbosity, CLAP_ACTION_COUNT);
    clap_argument_dest(verbosity, "debug_level");
    clap_argument_help(verbosity, "Increase debug level (-d, -dd, -ddd) (COUNT action)");
    
    /* ========================================================================
     * 4. APPEND action
     * ======================================================================== */
    clap_argument_t *tags = clap_add_argument(parser, "--tags/-t");
    clap_argument_action(tags, CLAP_ACTION_APPEND);
    clap_argument_type(tags, "string");
    clap_argument_help(tags, "Add a tag (can be used multiple times) (APPEND action)");
    clap_argument_metavar(tags, "TAG");
    
    clap_argument_t *numbers = clap_add_argument(parser, "--num");
    clap_argument_action(numbers, CLAP_ACTION_APPEND);
    clap_argument_type(numbers, "int");
    clap_argument_help(numbers, "Add a number (can be used multiple times) (APPEND action)");
    clap_argument_metavar(numbers, "N");
    
    /* ========================================================================
     * 5. STORE_CONST action
     * ======================================================================== */
    int mode_group = clap_add_mutually_exclusive_group(parser, false);
    
    clap_argument_t *mode_fast = clap_add_argument(parser, "--mode-fast");
    clap_argument_action(mode_fast, CLAP_ACTION_STORE_CONST);
    clap_argument_const(mode_fast, "fast");
    clap_argument_dest(mode_fast, "mode");
    clap_argument_help(mode_fast, "Set mode to 'fast' (STORE_CONST action)");
    clap_argument_mutex_group(mode_fast, mode_group);
    clap_mutex_group_add_argument(parser, mode_group, mode_fast);
    
    clap_argument_t *mode_slow = clap_add_argument(parser, "--mode-slow");
    clap_argument_action(mode_slow, CLAP_ACTION_STORE_CONST);
    clap_argument_const(mode_slow, "slow");
    clap_argument_dest(mode_slow, "mode");
    clap_argument_help(mode_slow, "Set mode to 'slow' (STORE_CONST action)");
    clap_argument_mutex_group(mode_slow, mode_group);
    clap_mutex_group_add_argument(parser, mode_group, mode_slow);
    
    clap_argument_t *mode_normal = clap_add_argument(parser, "--mode-normal");
    clap_argument_action(mode_normal, CLAP_ACTION_STORE_CONST);
    clap_argument_const(mode_normal, "normal");
    clap_argument_dest(mode_normal, "mode");
    clap_argument_default(mode_normal, "normal");
    clap_argument_help(mode_normal, "Set mode to 'normal' (default) (STORE_CONST action)");
    clap_argument_mutex_group(mode_normal, mode_group);
    clap_mutex_group_add_argument(parser, mode_group, mode_normal);
    
    /* ========================================================================
     * 6. APPEND_CONST action
     * ======================================================================== */
    clap_argument_t *add_item = clap_add_argument(parser, "--add-item");
    clap_argument_action(add_item, CLAP_ACTION_APPEND_CONST);
    clap_argument_const(add_item, "item");
    clap_argument_dest(add_item, "items");
    clap_argument_help(add_item, "Add 'item' to items list (APPEND_CONST action)");
    
    clap_argument_t *add_flag = clap_add_argument(parser, "--flag");
    clap_argument_action(add_flag, CLAP_ACTION_APPEND_CONST);
    clap_argument_const(add_flag, "FLAG");
    clap_argument_dest(add_flag, "flags");
    clap_argument_help(add_flag, "Add 'FLAG' to flags list (APPEND_CONST action)");
    
    /* ========================================================================
     * 7. CUSTOM action
     * ======================================================================== */
    clap_argument_t *to_upper = clap_add_argument(parser, "--to-upper");
    clap_argument_action(to_upper, CLAP_ACTION_CUSTOM);
    clap_argument_handler(to_upper, custom_uppercase_handler);
    clap_argument_type(to_upper, "string");
    clap_argument_dest(to_upper, "to_upper");
    clap_argument_help(to_upper, "Convert and store value in uppercase (CUSTOM action)");
    clap_argument_metavar(to_upper, "TEXT");
    
    /* ========================================================================
     * 8. HELP and VERSION actions
     * ======================================================================== */
    clap_argument_t *version_opt = clap_add_argument(parser, "--version/-V");
    clap_argument_action(version_opt, CLAP_ACTION_VERSION);
    clap_argument_help(version_opt, "Show version information (VERSION action)");
    
    /* ========================================================================
     * Parse arguments
     * ======================================================================== */
    bool parse_ok = clap_parse_args(parser, argc, argv, &ns, &error);
    
    if (!parse_ok) {
        const char *base_name = strrchr(argv[0], '/');
        if (!base_name) base_name = strrchr(argv[0], '\\');
        if (base_name) base_name++;
        else base_name = argv[0];
        
        fprintf(stderr, "%s: error: %s\n", base_name, error.message);
        fprintf(stderr, "\nTry '%s --help' for more information.\n", base_name);
        
        clap_parser_free(parser);
        return EXIT_FAILURE;
    }
    
    /* ========================================================================
    * Display results
    * ======================================================================== */
    printf("\n");
    printf("======================================================================\n");
    printf("                    PARSED ARGUMENTS SUMMARY                          \n");
    printf("======================================================================\n\n");

    /* STORE results */
    const char *name_val = NULL;
    if (clap_namespace_get_string(ns, "name", &name_val)) {
        printf("  Name (STORE):        %s\n", name_val);
    } else {
        printf("  Name (STORE):        (not specified)\n");
    }

    int age_val = 0;
    if (clap_namespace_get_int(ns, "age", &age_val)) {
        printf("  Age (STORE):         %d\n", age_val);
    } else {
        printf("  Age (STORE):         (not specified)\n");
    }

    /* STORE_TRUE / STORE_FALSE results */
    bool verbose_val = false;
    clap_namespace_get_bool(ns, "verbose", &verbose_val);
    printf("  Verbose (STORE_TRUE): %s\n", verbose_val ? "ON" : "OFF");

    /* COUNT results */
    int debug_level = 0;
    clap_namespace_get_int(ns, "debug_level", &debug_level);
    printf("  Debug level (COUNT): %d\n", debug_level);

    /* APPEND results */
    const char **tags_list;
    size_t tags_count = 0;
    if (clap_namespace_get_string_array(ns, "tags", &tags_list, &tags_count)) {
        printf("  Tags (APPEND):       [");
        for (size_t i = 0; i < tags_count; i++) {
            if (i > 0) printf(", ");
            printf("%s", tags_list[i]);
        }
        printf("] (%zu items)\n", tags_count);
    } else {
        printf("  Tags (APPEND):       [] (0 items)\n");
    }

    const char **nums_list;
    size_t nums_count = 0;
    if (clap_namespace_get_string_array(ns, "num", &nums_list, &nums_count)) {
        printf("  Numbers (APPEND):    [");
        for (size_t i = 0; i < nums_count; i++) {
            if (i > 0) printf(", ");
            printf("%s", nums_list[i]);
        }
        printf("] (%zu items)\n", nums_count);
    } else {
        printf("  Numbers (APPEND):    [] (0 items)\n");
    }

    /* STORE_CONST results */
    const char *mode_val = "normal";
    clap_namespace_get_string(ns, "mode", &mode_val);
    printf("  Mode (STORE_CONST):  %s\n", mode_val);

    /* APPEND_CONST results */
    const char **items_list;
    size_t items_count = 0;
    if (clap_namespace_get_string_array(ns, "items", &items_list, &items_count)) {
        printf("  Items (APPEND_CONST): [");
        for (size_t i = 0; i < items_count; i++) {
            if (i > 0) printf(", ");
            printf("%s", items_list[i]);
        }
        printf("] (%zu items)\n", items_count);
    } else {
        printf("  Items (APPEND_CONST): [] (0 items)\n");
    }

    const char **flags_list;
    size_t flags_count = 0;
    if (clap_namespace_get_string_array(ns, "flags", &flags_list, &flags_count)) {
        printf("  Flags (APPEND_CONST):[");
        for (size_t i = 0; i < flags_count; i++) {
            if (i > 0) printf(", ");
            printf("%s", flags_list[i]);
        }
        printf("] (%zu items)\n", flags_count);
    } else {
        printf("  Flags (APPEND_CONST):[] (0 items)\n");
    }

    /* CUSTOM results */
    const char *upper_val = NULL;
    if (clap_namespace_get_string(ns, "to_upper", &upper_val)) {
        printf("  Uppercase (CUSTOM):  %s\n", upper_val);
    } else {
        printf("  Uppercase (CUSTOM):  (not specified)\n");
    }

    printf("\n");
    
    /* ========================================================================
     * Cleanup
     * ======================================================================== */
    clap_namespace_free(ns);
    clap_parser_free(parser);
    
    return EXIT_SUCCESS;
}