/**
 * @file clap_action_executor.c
 * @brief Applies parsed argument actions to the namespace
 */

#include "clap_parser_internal.h"

bool clap_apply_argument_action(clap_parser_t *parser,
                                clap_argument_t *arg,
                                clap_namespace_t *ns,
                                const char *value,
                                clap_error_t *error) {
    if (!arg || !ns) {
        if (error) {
            clap_error_set(error, CLAP_ERR_INVALID_ARGUMENT,
                           "Invalid action application parameters");
        }
        return false;
    }

    if (value && arg->choices && arg->choice_count > 0) {
        if (!clap_validate_choice(arg, value, error)) {
            return false;
        }
    }

    const char *values[1] = { value };
    size_t count = value ? 1 : 0;

    clap_action_handler_t handler = arg->action_handler;
    if (!handler && arg->action != CLAP_ACTION_CUSTOM) {
        handler = get_action_handler(arg->action);
    }

    if (!handler) {
        clap_error_set(error, CLAP_ERR_INVALID_ARGUMENT,
                       arg->action == CLAP_ACTION_CUSTOM
                           ? "CUSTOM action requires action_handler"
                           : "Unknown action type");
        return false;
    }

    return handler(parser, arg, ns, values, count, arg->action_data, error);
}