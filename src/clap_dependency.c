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

static bool is_arg_present(clap_argument_t *arg, clap_namespace_t *ns) {
    if (!arg || !ns) return false;
    return clap_namespace_has(ns, clap_buffer_cstr(arg->dest));
}

bool clap_validate_dependencies(clap_parser_t *parser,
                                clap_namespace_t *ns,
                                clap_error_t *error) {
    if (!parser || !ns) return true;

    for (size_t i = 0; i < parser->arg_count; i++) {
        clap_argument_t *arg = parser->arguments[i];
        if (!arg || arg->dependency_count == 0) continue;
        if (!is_arg_present(arg, ns)) continue;

        for (size_t j = 0; j < arg->dependency_count; j++) {
            clap_dependency_t *dep = arg->dependencies[j];
            if (!dep) continue;

            bool target_present = is_arg_present(dep->targets[0], ns);

            if (dep->type == CLAP_DEP_REQUIRES && !target_present) {
                if (dep->error_message) {
                    clap_error_set(error, CLAP_ERR_DEPENDENCY_VIOLATION,
                                   "%s", dep->error_message);
                } else {
                    clap_error_set(error, CLAP_ERR_DEPENDENCY_VIOLATION,
                                   CLAP_TR("argument '%s' requires '%s'"),
                                   clap_buffer_cstr(arg->display_name),
                                   clap_buffer_cstr(dep->targets[0]->display_name));
                }
                return false;
            }

            if (dep->type == CLAP_DEP_CONFLICTS && target_present) {
                if (dep->error_message) {
                    clap_error_set(error, CLAP_ERR_DEPENDENCY_VIOLATION,
                                   "%s", dep->error_message);
                } else {
                    clap_error_set(error, CLAP_ERR_DEPENDENCY_VIOLATION,
                                   CLAP_TR("argument '%s' conflicts with '%s'"),
                                   clap_buffer_cstr(arg->display_name),
                                   clap_buffer_cstr(dep->targets[0]->display_name));
                }
                return false;
            }
        }
    }

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
