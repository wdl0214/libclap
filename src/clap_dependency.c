/**
* @file clap_dependency.c
 * @brief Argument dependency implementation
 */

#include "clap_parser_internal.h"

bool clap_argument_requires(clap_argument_t *arg,
                            clap_argument_t *required_arg,
                            const char *error_msg) {
    if (!arg || !required_arg) return false;

    if (arg->dependency_count >= CLAP_MAX_DEPENDENCIES) return false;

    if (!arg->dependencies) {
        arg->dependencies = clap_calloc(CLAP_MAX_DEPENDENCIES, sizeof(clap_dependency_t*));
        if (!arg->dependencies) return false;
    }

    clap_dependency_t *dep = clap_calloc(1, sizeof(clap_dependency_t));
    if (!dep) return false;

    dep->type = CLAP_DEP_REQUIRES;
    dep->source = arg;
    dep->target_count = 1;
    dep->targets = clap_calloc(1, sizeof(clap_argument_t*));
    dep->targets[0] = required_arg;

    if (error_msg) {
        dep->error_message = clap_strdup(error_msg);
    }

    arg->dependencies[arg->dependency_count++] = dep;
    return true;
}

bool clap_argument_conflicts(clap_argument_t *arg,
                              clap_argument_t *conflicting_arg,
                              const char *error_msg) {
    if (!arg || !conflicting_arg) return false;

    if (arg->dependency_count >= CLAP_MAX_DEPENDENCIES) return false;

    if (!arg->dependencies) {
        arg->dependencies = clap_calloc(CLAP_MAX_DEPENDENCIES, sizeof(clap_dependency_t*));
        if (!arg->dependencies) return false;
    }

    clap_dependency_t *dep = clap_calloc(1, sizeof(clap_dependency_t));
    if (!dep) return false;

    dep->type = CLAP_DEP_CONFLICTS;
    dep->source = arg;
    dep->target_count = 1;
    dep->targets = clap_calloc(1, sizeof(clap_argument_t*));
    dep->targets[0] = conflicting_arg;

    if (error_msg) {
        dep->error_message = clap_strdup(error_msg);
    }

    arg->dependencies[arg->dependency_count++] = dep;
    return true;
}
