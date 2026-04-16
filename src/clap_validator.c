/**
 * @file clap_validator.c
 * @brief Validation implementation
 */

#include "clap_parser_internal.h"

bool clap_validate_choice(clap_argument_t *arg, const char *value, clap_error_t *error) {
    if (!arg->choices || arg->choice_count == 0) return true;

    for (size_t i = 0; i < arg->choice_count; i++) {
        if (strcmp(arg->choices[i], value) == 0) return true;
    }

    clap_buffer_t *choices_str = clap_buffer_empty();
    for (size_t i = 0; i < arg->choice_count; i++) {
        if (i > 0) clap_buffer_cat(&choices_str, ", ");
        clap_buffer_cat(&choices_str, arg->choices[i]);
    }

    clap_error_set(error, CLAP_ERR_INVALID_CHOICE,
                   "Invalid choice '%s' (valid: %s)",
                   value, clap_buffer_cstr(choices_str));

    clap_buffer_free(choices_str);
    return false;
}

bool clap_validate_nargs(clap_argument_t *arg, size_t value_count, clap_error_t *error) {
    if (arg->nargs == CLAP_NARGS_ZERO_OR_ONE) {
        if (value_count > 1) {
            clap_error_set(error, CLAP_ERR_TOO_MANY_ARGS,
                           "Expected 0-1 arguments, got %zu", value_count);
            return false;
        }
    } else if (arg->nargs == CLAP_NARGS_ZERO_OR_MORE) {
        return true;
    } else if (arg->nargs == CLAP_NARGS_ONE_OR_MORE) {
        if (value_count < 1) {
            clap_error_set(error, CLAP_ERR_TOO_FEW_ARGS,
                           "Expected at least 1 argument");
            return false;
        }
    } else if (arg->nargs == CLAP_NARGS_REMAINDER) {
        return true;
    } else if (arg->nargs > 0) {
        if (value_count != (size_t)arg->nargs) {
            clap_error_set(error,
                value_count < (size_t)arg->nargs ? CLAP_ERR_TOO_FEW_ARGS : CLAP_ERR_TOO_MANY_ARGS,
                "Expected %d argument(s), got %zu", arg->nargs, value_count);
            return false;
        }
    }

    return true;
}

bool clap_argument_validate(clap_argument_t *arg, clap_error_t *error) {
    if ((arg->flags & CLAP_ARG_POSITIONAL) && arg->nargs < 1) {
        clap_error_set(error, CLAP_ERR_INVALID_ARGUMENT,
                       "Positional argument cannot have nargs < 1");
        return false;
    }
    
    /* Set action handler based on action type */
    if (arg->action != CLAP_ACTION_CUSTOM) {
        arg->action_handler = get_action_handler(arg->action);
        if (!arg->action_handler) {
            clap_error_set(error, CLAP_ERR_INVALID_ARGUMENT,
                           "Unknown action type");
            return false;
        }
    }
    
    return true;
}