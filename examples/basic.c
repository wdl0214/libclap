/**
* @example basic.c
 * @brief Basic usage example
 */

#include <clap/clap.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    clap_error_t error = {0};
    clap_namespace_t *ns = NULL;
    clap_argument_t *arg;

    clap_parser_t *parser = clap_parser_new(
        "basic",
        "Basic example of libclap",
        "Report bugs to: dev@example.com"
    );

    /* Positional argument */
    arg = clap_add_argument(parser, "input");
    clap_argument_help(arg, "Input file");
    clap_argument_type(arg, "string");
    clap_argument_required(arg, true);

    /* Optional argument */
    arg = clap_add_argument(parser, "--output/-o");
    clap_argument_help(arg, "Output file");
    clap_argument_type(arg, "string");
    clap_argument_default(arg, "-");

    /* Flag argument */
    arg = clap_add_argument(parser, "--verbose/-v");
    clap_argument_help(arg, "Verbose output");
    clap_argument_action(arg, CLAP_ACTION_COUNT);

    if (!clap_parse_args(parser, argc, argv, &ns, &error)) {
        fprintf(stderr, "Error: %s\n", error.message);
        clap_print_help(parser, stderr);

        clap_parser_free(parser);
        return EXIT_FAILURE;
    }

    const char *input, *output;
    int verbose = 0;

    clap_namespace_get_string(ns, "input", &input);
    clap_namespace_get_string(ns, "output", &output);
    clap_namespace_get_int(ns, "verbose", &verbose);

    printf("Input: %s\n", input);
    printf("Output: %s\n", output);
    printf("Verbose: %d\n", verbose);

    clap_namespace_free(ns);
    clap_parser_free(parser);

    return EXIT_SUCCESS;
}