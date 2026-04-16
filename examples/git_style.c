/**
 * @file git_style.c
 * @brief Git-style command line interface using libclap
 * 
 * Demonstrates:
 * - Subcommands (commit, add, status)
 * - Mutually exclusive groups (--quiet vs --verbose)
 * - nargs='+' for multiple files
 * - Flag arguments (--amend, --all)
 * - Short and long options
 * - Flat namespace access (same as Python argparse)
 * 
 * Compile:
 * $ gcc -Iinclude -Lbuild/src git_style.c -lclap -o git_demo
 * 
 * Usage examples:
 * $ ./git_demo commit -m "Initial commit"
 * $ ./git_demo commit --amend --message "Fix typo"
 * $ ./git_demo add file1.txt file2.txt file3.txt
 * $ ./git_demo add --all --verbose
 * $ ./git_demo status --short
 * $ ./git_demo --help
 */

#include <clap/clap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Function prototypes */
int handle_commit(clap_namespace_t *ns);
int handle_add(clap_namespace_t *ns);
int handle_status(clap_namespace_t *ns);
void print_error(const clap_error_t *error, const char *prog_name);

int main(int argc, char *argv[]) {
    clap_error_t error = {0};
    clap_namespace_t *ns = NULL;
    int exit_code = EXIT_SUCCESS;
    clap_argument_t *arg;
    
    /* ====================================================================
     * 1. Create main parser
     * ==================================================================== */
    clap_parser_t *parser = clap_parser_new(
        "git_demo",
        "Git-style version control system demo using libclap",
        "Examples:\n"
        "  git_demo commit -m \"message\"\n"
        "  git_demo add file1.txt file2.txt\n"
        "  git_demo status --short"
    );
    
    if (!parser) {
        fprintf(stderr, "Failed to create parser\n");
        return EXIT_FAILURE;
    }
    
    clap_parser_set_help_width(parser, 200);
    clap_parser_set_version(parser, "1.0.1");
    
    /* ====================================================================
     * 2. Add global options
     * ==================================================================== */
    arg = clap_add_argument(parser, "--version/-V");
    clap_argument_action(arg, CLAP_ACTION_VERSION);
    clap_argument_help(arg, "Show version information");
    
    /* ====================================================================
     * 3. Create subparsers
     * ==================================================================== */
    clap_parser_t *subparsers = clap_add_subparsers(parser, "command", "Available commands");
    if (!subparsers) {
        fprintf(stderr, "Failed to create subparsers\n");
        clap_parser_free(parser);
        return EXIT_FAILURE;
    }
    
    /* --------------------------------------------------------------------
     * Subcommand: commit
     * -------------------------------------------------------------------- */
    clap_parser_t *commit_parser = clap_subparser_add(subparsers, "commit", 
                                                       "Record changes to the repository");
    if (!commit_parser) {
        fprintf(stderr, "Failed to create commit subparser\n");
        clap_parser_free(parser);
        return EXIT_FAILURE;
    }
    
    arg = clap_add_argument(commit_parser, "--message/-m");
    clap_argument_type(arg, "string");
    clap_argument_help(arg, "Commit message");
    clap_argument_required(arg, true);
    clap_argument_metavar(arg, "MSG");
    
    arg = clap_add_argument(commit_parser, "--amend");
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    clap_argument_help(arg, "Amend previous commit");
    
    arg = clap_add_argument(commit_parser, "--author");
    clap_argument_type(arg, "string");
    clap_argument_help(arg, "Override commit author");
    clap_argument_metavar(arg, "NAME");
    
    arg = clap_add_argument(commit_parser, "--date");
    clap_argument_type(arg, "string");
    clap_argument_help(arg, "Override commit date");
    clap_argument_metavar(arg, "DATE");
    
    /* Mutually exclusive group: quiet vs verbose */
    int commit_verbosity_group = clap_add_mutually_exclusive_group(commit_parser, false);
    
    arg = clap_add_argument(commit_parser, "--quiet/-q");
    clap_argument_group(arg, commit_verbosity_group);
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    clap_argument_help(arg, "Suppress commit summary");
    clap_mutex_group_add_argument(commit_parser, commit_verbosity_group, arg);
    
    arg = clap_add_argument(commit_parser, "--verbose/-v");
    clap_argument_group(arg, commit_verbosity_group);
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    clap_argument_help(arg, "Show unified diff in commit template");
    clap_mutex_group_add_argument(commit_parser, commit_verbosity_group, arg);
    
    arg = clap_add_argument(commit_parser, "files");
    clap_argument_nargs(arg, '*');
    clap_argument_type(arg, "string");
    clap_argument_help(arg, "Files to commit (default: all staged)");
    clap_argument_metavar(arg, "FILE");
    
    /* --------------------------------------------------------------------
     * Subcommand: add
     * -------------------------------------------------------------------- */
    clap_parser_t *add_parser = clap_subparser_add(subparsers, "add", 
                                                    "Add file contents to the index");
    if (!add_parser) {
        fprintf(stderr, "Failed to create add subparser\n");
        clap_parser_free(parser);
        return EXIT_FAILURE;
    }
    
    arg = clap_add_argument(add_parser, "--all/-A");
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    clap_argument_help(arg, "Add all files, including untracked");
    
    arg = clap_add_argument(add_parser, "--update/-u");
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    clap_argument_help(arg, "Update tracked files");
    
    arg = clap_add_argument(add_parser, "--force/-f");
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    clap_argument_help(arg, "Allow adding ignored files");
    
    int add_verbosity_group = clap_add_mutually_exclusive_group(add_parser, false);
    
    arg = clap_add_argument(add_parser, "--quiet/-q");
    clap_argument_group(arg, add_verbosity_group);
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    clap_argument_help(arg, "Be quiet");
    clap_mutex_group_add_argument(add_parser, add_verbosity_group, arg);
    
    arg = clap_add_argument(add_parser, "--verbose/-v");
    clap_argument_group(arg, add_verbosity_group);
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    clap_argument_help(arg, "Be verbose");
    clap_mutex_group_add_argument(add_parser, add_verbosity_group, arg);
    
    arg = clap_add_argument(add_parser, "--dry-run/-n");
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    clap_argument_help(arg, "Don't actually add files, just show what would be done");
    
    arg = clap_add_argument(add_parser, "files");
    clap_argument_nargs(arg, '+');
    clap_argument_type(arg, "string");
    clap_argument_help(arg, "Files to add");
    clap_argument_metavar(arg, "FILE");
    
    /* --------------------------------------------------------------------
     * Subcommand: status
     * -------------------------------------------------------------------- */
    clap_parser_t *status_parser = clap_subparser_add(subparsers, "status", 
                                                       "Show the working tree status");
    if (!status_parser) {
        fprintf(stderr, "Failed to create status subparser\n");
        clap_parser_free(parser);
        return EXIT_FAILURE;
    }
    
    arg = clap_add_argument(status_parser, "--short/-s");
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    clap_argument_help(arg, "Give output in short format");
    
    arg = clap_add_argument(status_parser, "--branch/-b");
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    clap_argument_help(arg, "Show branch information");
    
    arg = clap_add_argument(status_parser, "--porcelain/-p");
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    clap_argument_help(arg, "Machine-readable output");
    
    arg = clap_add_argument(status_parser, "--untracked-files/-u");
    clap_argument_type(arg, "string");
    clap_argument_choices(arg, (const char*[]){"no", "normal", "all"}, 3);
    clap_argument_default(arg, "normal");
    clap_argument_help(arg, "Show untracked files");
    
    arg = clap_add_argument(status_parser, "--ignored");
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    clap_argument_help(arg, "Show ignored files");
    
    int status_format_group = clap_add_mutually_exclusive_group(status_parser, false);
    
    arg = clap_add_argument(status_parser, "--json");
    clap_argument_group(arg, status_format_group);
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    clap_argument_help(arg, "Output in JSON format");
    clap_mutex_group_add_argument(status_parser, status_format_group, arg);
    
    arg = clap_add_argument(status_parser, "--xml");
    clap_argument_group(arg, status_format_group);
    clap_argument_action(arg, CLAP_ACTION_STORE_TRUE);
    clap_argument_help(arg, "Output in XML format");
    clap_mutex_group_add_argument(status_parser, status_format_group, arg);
    
    arg = clap_add_argument(status_parser, "pathspec");
    clap_argument_nargs(arg, '*');
    clap_argument_type(arg, "string");
    clap_argument_help(arg, "Limit to specific paths");
    clap_argument_metavar(arg, "PATH");
    
    /* ====================================================================
     * 4. Parse arguments
     * ==================================================================== */
    bool parse_ok = clap_parse_args(parser, argc, argv, &ns, &error);
    
    if (!parse_ok) {
        const char *base_name = strrchr(argv[0], '/');
        if (!base_name) base_name = strrchr(argv[0], '\\');
        if (base_name) base_name++;
        else base_name = argv[0];
        
        if (error.subcommand_name) {
            fprintf(stderr, "%s %s: error: %s\n\n", 
                    base_name, error.subcommand_name, error.message);
            clap_print_subcommand_help(parser, error.subcommand_name, stderr);
        } else {
            fprintf(stderr, "%s: error: %s\n", base_name, error.message);
            fprintf(stderr, "\nTry '%s --help' for more information.\n", base_name);
        }
        exit_code = EXIT_FAILURE;
        goto cleanup_parser;
    }
    
    /* ====================================================================
     * 5. Route to appropriate subcommand handler
     * Flat namespace - same as Python argparse!
     * ==================================================================== */
    const char *command;
    if (clap_namespace_get_string(ns, "command", &command)) {
        if (strcmp(command, "commit") == 0) {
            exit_code = handle_commit(ns);
        } else if (strcmp(command, "add") == 0) {
            exit_code = handle_add(ns);
        } else if (strcmp(command, "status") == 0) {
            exit_code = handle_status(ns);
        } else {
            fprintf(stderr, "Unknown command: %s\n", command);
            exit_code = EXIT_FAILURE;
        }
    } else {
        /* No subcommand specified - show help */
        clap_print_help(parser, stdout);
        exit_code = EXIT_SUCCESS;
    }
    
    clap_namespace_free(ns);
    
cleanup_parser:
    clap_parser_free(parser);
    
    return exit_code;
}

/* ========================================================================
 * Print Error
 * ======================================================================== */
void print_error(const clap_error_t *error, const char *prog_name) {
    const char *base_name = strrchr(prog_name, '/');
    if (!base_name) base_name = strrchr(prog_name, '\\');
    if (base_name) base_name++;
    else base_name = prog_name;
    
    fprintf(stderr, "%s: error: %s\n", base_name, error->message);
}

/* ========================================================================
 * Command Handlers - All values accessed directly from flat namespace!
 * ======================================================================== */

int handle_commit(clap_namespace_t *ns) {
    printf("\033[1;36m=== COMMIT ===\033[0m\n");
    
    /* Access values directly - flat namespace! */
    const char *message;
    clap_namespace_get_string(ns, "message", &message);
    printf("Message: %s\n", message);
    
    bool amend;
    clap_namespace_get_bool(ns, "amend", &amend);
    printf("Amend: %s\n", amend ? "yes" : "no");
    
    const char *author;
    if (clap_namespace_get_string(ns, "author", &author)) {
        printf("Author: %s\n", author);
    }
    
    const char *date;
    if (clap_namespace_get_string(ns, "date", &date)) {
        printf("Date: %s\n", date);
    }
    
    bool quiet, verbose;
    clap_namespace_get_bool(ns, "quiet", &quiet);
    clap_namespace_get_bool(ns, "verbose", &verbose);
    
    if (quiet) {
        printf("Verbosity: quiet\n");
    } else if (verbose) {
        printf("Verbosity: verbose\n");
    } else {
        printf("Verbosity: normal\n");
    }
    
    const char **files;
    size_t file_count;
    if (clap_namespace_get_string_array(ns, "files", &files, &file_count)) {
        if (file_count > 0) {
            printf("Files to commit (%zu):\n", file_count);
            for (size_t i = 0; i < file_count; i++) {
                printf("  - %s\n", files[i]);
            }
        } else {
            printf("Files: All staged files\n");
        }
    }
    
    printf("\n\033[1;32m✓ Commit created successfully!\033[0m\n");
    return EXIT_SUCCESS;
}

int handle_add(clap_namespace_t *ns) {
    printf("\033[1;36m=== ADD ===\033[0m\n");
    
    /* Access values directly - flat namespace! */
    bool all, update, force, dry_run;
    clap_namespace_get_bool(ns, "all", &all);
    clap_namespace_get_bool(ns, "update", &update);
    clap_namespace_get_bool(ns, "force", &force);
    clap_namespace_get_bool(ns, "dry_run", &dry_run);
    
    printf("Options:\n");
    printf("  --all: %s\n", all ? "yes" : "no");
    printf("  --update: %s\n", update ? "yes" : "no");
    printf("  --force: %s\n", force ? "yes" : "no");
    printf("  --dry-run: %s\n", dry_run ? "yes" : "no");
    
    bool quiet, verbose;
    clap_namespace_get_bool(ns, "quiet", &quiet);
    clap_namespace_get_bool(ns, "verbose", &verbose);
    
    if (quiet) {
        printf("  Verbosity: quiet\n");
    } else if (verbose) {
        printf("  Verbosity: verbose\n");
    }
    
    const char **files;
    size_t file_count;
    clap_namespace_get_string_array(ns, "files", &files, &file_count);
    
    printf("\nFiles to add (%zu):\n", file_count);
    for (size_t i = 0; i < file_count; i++) {
        printf("  \033[1;32m✓\033[0m %s\n", files[i]);
    }
    
    if (dry_run) {
        printf("\n\033[1;33m[DRY RUN] No files were actually added.\033[0m\n");
    } else {
        printf("\n\033[1;32m✓ %zu file(s) added successfully!\033[0m\n", file_count);
    }
    
    return EXIT_SUCCESS;
}

int handle_status(clap_namespace_t *ns) {
    printf("\033[1;36m=== STATUS ===\033[0m\n");
    
    /* Access values directly - flat namespace! */
    bool short_fmt, branch, porcelain, ignored;
    clap_namespace_get_bool(ns, "short", &short_fmt);
    clap_namespace_get_bool(ns, "branch", &branch);
    clap_namespace_get_bool(ns, "porcelain", &porcelain);
    clap_namespace_get_bool(ns, "ignored", &ignored);
    
    const char *untracked;
    clap_namespace_get_string(ns, "untracked_files", &untracked);
    
    bool json, xml;
    clap_namespace_get_bool(ns, "json", &json);
    clap_namespace_get_bool(ns, "xml", &xml);
    
    printf("Format: ");
    if (short_fmt) printf("short");
    else if (porcelain) printf("porcelain");
    else if (json) printf("JSON");
    else if (xml) printf("XML");
    else printf("long");
    printf("\n");
    
    printf("Show branch: %s\n", branch ? "yes" : "no");
    printf("Untracked files: %s\n", untracked);
    printf("Show ignored: %s\n", ignored ? "yes" : "no");
    
    const char **paths;
    size_t path_count;
    if (clap_namespace_get_string_array(ns, "pathspec", &paths, &path_count)) {
        if (path_count > 0) {
            printf("\nPathspec (%zu):\n", path_count);
            for (size_t i = 0; i < path_count; i++) {
                printf("  - %s\n", paths[i]);
            }
        }
    }
    
    printf("\n\033[1;32mOn branch main\033[0m\n");
    printf("Changes to be committed:\n");
    printf("  \033[1;32mnew file:   README.md\033[0m\n");
    printf("  \033[1;32mmodified:   src/main.c\033[0m\n");
    
    return EXIT_SUCCESS;
}