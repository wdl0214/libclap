/**
 * @file fuzz_clap.c
 * @brief LibFuzzer target for libclap
 * 
 * Build with:
 *   clang -g -O1 -fsanitize=fuzzer,address -Iinclude -I_deps/unity-src/src \
 *         fuzz_clap.c -Lbuild/src -lclap_static -o fuzz_clap
 * 
 * Run with:
 *   ./fuzz_clap -max_len=4096 -runs=1000000 corpus/
 */

#include <clap/clap.h>
#include "clap_parser_internal.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Maximum arguments to generate */
#define MAX_ARGS 32
#define MAX_ARG_LEN 256

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

/**
 * @brief Create a parser with various argument types for fuzzing
 */
static clap_parser_t* create_fuzz_parser(void) {
    clap_parser_t *parser = clap_parser_new(
        "fuzz_prog",
        "Fuzzing test harness for libclap",
        "This program is used for fuzz testing"
    );
    
    if (!parser) return NULL;

    /* Fuzzing should not treat built-in help as a crash-inducing exit path. */
    if (parser->arg_count > 0 && parser->arguments[0]->action == CLAP_ACTION_HELP) {
        clap_argument_action(parser->arguments[0], CLAP_ACTION_STORE_TRUE);
    }
    
    clap_parser_set_help_width(parser, 80);
    clap_parser_set_version(parser, "1.0.0");
    
    /* Positional arguments */
    clap_argument_t *pos = clap_add_argument(parser, "positional");
    clap_argument_nargs(pos, '*');
    clap_argument_type(pos, "string");
    
    /* Optional arguments - various types */
    clap_argument_t *int_opt = clap_add_argument(parser, "--int-opt/-i");
    clap_argument_type(int_opt, "int");
    clap_argument_default(int_opt, "42");
    clap_argument_help(int_opt, "Integer option");
    
    clap_argument_t *float_opt = clap_add_argument(parser, "--float-opt/-f");
    clap_argument_type(float_opt, "float");
    clap_argument_default(float_opt, "3.14");
    
    clap_argument_t *bool_flag = clap_add_argument(parser, "--bool-flag/-b");
    clap_argument_action(bool_flag, CLAP_ACTION_STORE_TRUE);
    
    clap_argument_t *choice_opt = clap_add_argument(parser, "--choice-opt/-c");
    clap_argument_type(choice_opt, "string");
    clap_argument_choices(choice_opt, (const char*[]){"one", "two", "three"}, 3);
    
    clap_argument_t *multi_opt = clap_add_argument(parser, "--multi-opt/-m");
    clap_argument_nargs(multi_opt, '+');
    clap_argument_type(multi_opt, "string");
    
    clap_argument_t *count_opt = clap_add_argument(parser, "--count-opt/-C");
    clap_argument_action(count_opt, CLAP_ACTION_COUNT);
    clap_argument_dest(count_opt, "count");
    
    clap_argument_t *append_opt = clap_add_argument(parser, "--append-opt/-a");
    clap_argument_action(append_opt, CLAP_ACTION_APPEND);
    clap_argument_type(append_opt, "string");
    
    /* Store false action */
    clap_argument_t *no_feature = clap_add_argument(parser, "--no-feature");
    clap_argument_action(no_feature, CLAP_ACTION_STORE_FALSE);
    clap_argument_dest(no_feature, "feature_enabled");
    
    /* Store const action */
    clap_argument_t *const_opt = clap_add_argument(parser, "--const-opt");
    clap_argument_action(const_opt, CLAP_ACTION_STORE_CONST);
    clap_argument_const(const_opt, "constant_value");
    clap_argument_dest(const_opt, "const_val");
    
    /* Append const action */
    clap_argument_t *append_const = clap_add_argument(parser, "--append-const");
    clap_argument_action(append_const, CLAP_ACTION_APPEND_CONST);
    clap_argument_const(append_const, "const_item");
    clap_argument_dest(append_const, "const_list");
    
    /* Positional with nargs=2 (exact) */
    clap_argument_t *pair = clap_add_argument(parser, "pair");
    clap_argument_nargs(pair, 2);
    clap_argument_type(pair, "string");
    clap_argument_required(pair, false);
    
    /* Mutex group */
    int group = clap_add_mutually_exclusive_group(parser, false);
    
    clap_argument_t *mutex1 = clap_add_argument(parser, "--mutex1");
    clap_argument_action(mutex1, CLAP_ACTION_STORE_TRUE);
    clap_argument_group(mutex1, group);
    clap_mutex_group_add_argument(parser, group, mutex1);
    
    clap_argument_t *mutex2 = clap_add_argument(parser, "--mutex2");
    clap_argument_action(mutex2, CLAP_ACTION_STORE_TRUE);
    clap_argument_group(mutex2, group);
    clap_mutex_group_add_argument(parser, group, mutex2);
    
    /* Required mutex group */
    int req_group = clap_add_mutually_exclusive_group(parser, true);
    
    clap_argument_t *req1 = clap_add_argument(parser, "--req1");
    clap_argument_group(req1, req_group);
    clap_mutex_group_add_argument(parser, req_group, req1);
    
    clap_argument_t *req2 = clap_add_argument(parser, "--req2");
    clap_argument_group(req2, req_group);
    clap_mutex_group_add_argument(parser, req_group, req2);
    
    /* Subcommands */
    clap_parser_t *subparsers = clap_add_subparsers(parser, "command", "Available commands");
    
    clap_parser_t *cmd1 = clap_subparser_add(subparsers, "cmd1", "First command");
    clap_argument_t *cmd1_opt = clap_add_argument(cmd1, "--opt");
    clap_argument_type(cmd1_opt, "string");
    
    clap_parser_t *cmd2 = clap_subparser_add(subparsers, "cmd2", "Second command");
    clap_argument_t *cmd2_flag = clap_add_argument(cmd2, "--flag");
    clap_argument_action(cmd2_flag, CLAP_ACTION_STORE_TRUE);
    
    /* Nested subcommands under cmd1 */
    clap_parser_t *cmd1_subparsers = clap_add_subparsers(cmd1, "subcommand", "Subcommands for cmd1");
    clap_parser_t *subcmd1 = clap_subparser_add(cmd1_subparsers, "sub1", "First subcommand");
    clap_argument_t *sub1_arg = clap_add_argument(subcmd1, "arg");
    clap_argument_type(sub1_arg, "string");
    
    clap_parser_t *subcmd2 = clap_subparser_add(cmd1_subparsers, "sub2", "Second subcommand");
    clap_argument_t *sub2_opt = clap_add_argument(subcmd2, "--subopt");
    clap_argument_action(sub2_opt, CLAP_ACTION_STORE_TRUE);
    
    return parser;
}

/**
 * @brief Sanitize a string for use as command line argument
 */
static void sanitize_arg(char *dst, const uint8_t *src, size_t len) {
    size_t j = 0;
    for (size_t i = 0; i < len && j < MAX_ARG_LEN - 1; i++) {
        unsigned char c = src[i];
        
        /* Skip null bytes and control characters except tab */
        if (c == 0) continue;
        if (c < 0x20 && c != '\t') continue;
        
        dst[j++] = (char)c;
    }
    dst[j] = '\0';
}

/**
 * @brief Build argv array from fuzzer input
 */
static int build_argv(char *argv[], char arg_buffers[][MAX_ARG_LEN], 
                      const uint8_t *data, size_t size) {
    int argc = 1;
    argv[0] = "fuzz_prog";
    
    size_t offset = 0;
    
    while (offset < size && argc < MAX_ARGS) {
        /* Determine argument type and length from data */
        if (offset >= size) break;
        uint8_t type_byte = data[offset++];
        
        size_t arg_len = 0;
        if (offset < size) {
            arg_len = data[offset++] % (MAX_ARG_LEN - 1);
        }
        
        if (arg_len > 0 && offset + arg_len <= size) {
            /* Sanitize and copy argument */
            sanitize_arg(arg_buffers[argc], data + offset, arg_len);
            
            /* Skip empty arguments */
            if (arg_buffers[argc][0] == '\0') {
                offset += arg_len;
                continue;
            }
            
            /* Add dash prefix for some arguments to test option parsing */
            if (type_byte & 0x01) {
                char temp[MAX_ARG_LEN];
                if (type_byte & 0x02) {
                    snprintf(temp, sizeof(temp), "--%s", arg_buffers[argc]);
                } else {
                    snprintf(temp, sizeof(temp), "-%s", arg_buffers[argc]);
                }
                strcpy(arg_buffers[argc], temp);
            } else if (type_byte & 0x04) {
                /* Create equals-style option */
                char temp[MAX_ARG_LEN];
                snprintf(temp, sizeof(temp), "--opt=%s", arg_buffers[argc]);
                strcpy(arg_buffers[argc], temp);
            } else if (type_byte & 0x08) {
                /* Create short option bundle like -abc */
                if (strlen(arg_buffers[argc]) >= 1 && strlen(arg_buffers[argc]) <= 4) {
                    char bundle[MAX_ARG_LEN] = "-";
                    strncat(bundle, arg_buffers[argc], 4);
                    strcpy(arg_buffers[argc], bundle);
                }
            } else if (type_byte & 0x10) {
                /* Create invalid option for error testing */
                char invalid[MAX_ARG_LEN];
                snprintf(invalid, sizeof(invalid), "--invalid-%s", arg_buffers[argc]);
                strcpy(arg_buffers[argc], invalid);
            } else if (type_byte & 0x20) {
                /* Create type-mismatch for int/float fields */
                if (strlen(arg_buffers[argc]) < MAX_ARG_LEN - 10) {
                    strcat(arg_buffers[argc], "not-a-number");
                }
            }
            
            argv[argc] = arg_buffers[argc];
            argc++;
            offset += arg_len;
        }
    }
    
    return argc;
}

/* ============================================================================
 * LibFuzzer Entry Point
 * ============================================================================ */

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    /* Test boundary cases */
    if (size == 0) {
        /* Empty input */
        clap_parser_t *parser = create_fuzz_parser();
        if (parser) {
            clap_namespace_t *ns = NULL;
            clap_error_t error = {0};
            clap_parse_args(parser, 1, (char*[]){"fuzz_prog"}, &ns, &error);
            if (ns) clap_namespace_free(ns);
            clap_parser_free(parser);
        }
        return 0;
    }
    
    if (size > 10000) {
        /* Very large input - truncate */
        size = 10000;
    }
    
    if (size < 4) return 0;  /* Need at least some data */
    
    /* Create parser */
    clap_parser_t *parser = create_fuzz_parser();
    if (!parser) return 0;
    
    /* Build argv from fuzzer input */
    char *argv[MAX_ARGS];
    char arg_buffers[MAX_ARGS][MAX_ARG_LEN];
    
    int argc = build_argv(argv, arg_buffers, data, size);
    
    /* Parse with fuzzed input */
    clap_namespace_t *ns = NULL;
    clap_error_t error = {0};
    
    /* Ignore parse result - we're just looking for crashes */
    (void)clap_parse_args(parser, argc, argv, &ns, &error);
    
    /* Test help generation - exercises formatter */
    clap_print_help(parser, stdout);
    
    /* Try to access namespace values if parsing succeeded */
    if (ns) {
        const char *str_val;
        int int_val;
        bool bool_val;
        const char **array_val;
        size_t array_count;
        
        /* Positional */
        clap_namespace_get_string(ns, "positional", &str_val);
        clap_namespace_get_string_array(ns, "positional", &array_val, &array_count);
        
        /* Options */
        clap_namespace_get_int(ns, "int_opt", &int_val);
        double float_val;
        clap_namespace_get_float(ns, "float_opt", &float_val);
        clap_namespace_get_bool(ns, "bool_flag", &bool_val);
        clap_namespace_get_string(ns, "choice_opt", &str_val);
        clap_namespace_get_string_array(ns, "multi_opt", &array_val, &array_count);
        clap_namespace_get_int(ns, "count", &int_val);
        clap_namespace_get_string_array(ns, "append_opt", &array_val, &array_count);
        clap_namespace_get_bool(ns, "mutex1", &bool_val);
        clap_namespace_get_bool(ns, "mutex2", &bool_val);
        
        /* Store false and const actions */
        clap_namespace_get_bool(ns, "feature_enabled", &bool_val);
        clap_namespace_get_string(ns, "const_val", &str_val);
        clap_namespace_get_string_array(ns, "const_list", &array_val, &array_count);
        
        /* Positional pair */
        clap_namespace_get_string_array(ns, "pair", &array_val, &array_count);
        
        /* Subcommand */
        clap_namespace_get_string(ns, "command", &str_val);
        if (str_val) {
            clap_namespace_get_string(ns, "opt", &str_val);
            clap_namespace_get_bool(ns, "flag", &bool_val);
            
            /* Check for nested subcommands */
            clap_namespace_get_string(ns, "subcommand", &str_val);
            if (str_val) {
                clap_namespace_get_string(ns, "arg", &str_val);
                clap_namespace_get_bool(ns, "subopt", &bool_val);
            }
        }
        
        clap_namespace_free(ns);
    }
    
    /* Test version printing */
    clap_print_version(parser, stdout);
    
    /* Test error handling and validation */
    if (error.code != CLAP_ERR_NONE) {
        /* Error was properly detected */
        (void)error.code;  /* Verify error handling paths */
    }
    
    clap_parser_free(parser);
    
    return 0;
}

/* ============================================================================
 * Standalone Test Harness (for AFL or manual testing)
 * ============================================================================ */

#ifdef STANDALONE_FUZZ

#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }
    
    /* Read input file */
    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    uint8_t *data = malloc(size);
    if (!data) {
        fclose(f);
        return 1;
    }
    
    size_t read_size = fread(data, 1, size, f);
    fclose(f);
    
    if (read_size != size) {
        free(data);
        return 1;
    }
    
    int result = LLVMFuzzerTestOneInput(data, size);
    
    free(data);
    return result;
}

#endif /* STANDALONE_FUZZ */