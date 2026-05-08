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
    clap_parser_set_color(parser, true);

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

    /* Deprecated argument */
    arg = clap_add_argument(parser, "--format/-f");
    clap_argument_help(arg, "Output format");
    clap_argument_type(arg, "string");
    clap_argument_deprecated(arg, "use --output instead");

    clap_parse_result_t parse_result = clap_parse_args(parser, argc, argv, &ns, &error);
    if (parse_result == CLAP_PARSE_ERROR) {
        clap_print_help_on_error(parser, &error, stderr);

        clap_parser_free(parser);
        return EXIT_FAILURE;
    }
    if (parse_result == CLAP_PARSE_HELP || parse_result == CLAP_PARSE_VERSION) {
        clap_parser_free(parser);
        return EXIT_SUCCESS;
    }

    const char *input, *output, *format;
    int verbose = 0;

    clap_namespace_get_string(ns, "input", &input);
    clap_namespace_get_string(ns, "output", &output);
    clap_namespace_get_string(ns, "format", &format);
    clap_namespace_get_int(ns, "verbose", &verbose);

    printf("Input: %s\n", input);
    printf("Output: %s\n", output);
    if (format) printf("Format: %s\n", format);
    printf("Verbose: %d\n", verbose);

    clap_namespace_free(ns);
    clap_parser_free(parser);

    return EXIT_SUCCESS;
}
