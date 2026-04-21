/**
* @file clap_mutex.c
 * @brief Mutually exclusive groups implementation
 */

#include "clap_parser_internal.h"

int clap_add_mutually_exclusive_group(clap_parser_t *parser, bool required) {
    if (!parser) return -1;

    clap_mutex_group_t *group = clap_calloc(1, sizeof(clap_mutex_group_t));
    if (!group) return -1;

    group->id = parser->next_group_id++;
    group->required = required;
    group->arg_capacity = 4;
    group->arguments = clap_calloc(group->arg_capacity, sizeof(clap_argument_t*));

    if (!group->arguments) {
        clap_free(group);
        return -1;
    }

    clap_mutex_group_t **new_groups = clap_realloc(
        parser->mutex_groups,
        (parser->mutex_group_count + 1) * sizeof(clap_mutex_group_t*)
    );

    if (!new_groups) {
        clap_free(group->arguments);
        clap_free(group);
        return -1;
    }

    parser->mutex_groups = new_groups;
    parser->mutex_groups[parser->mutex_group_count++] = group;

    return group->id;
}

bool clap_mutex_group_add_argument(clap_parser_t *parser, int group_id, clap_argument_t *arg) {
    if (!parser || group_id < 0 || !arg) return false;
    
    clap_mutex_group_t *group = NULL;
    for (size_t i = 0; i < parser->mutex_group_count; i++) {
        if (parser->mutex_groups[i]->id == group_id) {
            group = parser->mutex_groups[i];
            break;
        }
    }
    
    if (!group) return false;
    
    /* Expand if needed */
    if (group->arg_count >= group->arg_capacity) {
        size_t new_cap = group->arg_capacity * 2;
        if (new_cap == 0) new_cap = 4;
        clap_argument_t **new_args = clap_realloc(
            group->arguments, new_cap * sizeof(clap_argument_t*)
        );
        if (!new_args) return false;
        group->arguments = new_args;
        group->arg_capacity = new_cap;
    }
    
    group->arguments[group->arg_count++] = arg;
    arg->group_id = group_id;
    return true;
}

/* Check for mutually exclusive group conflicts */
bool clap_mutex_check_conflict(clap_parser_t *parser,
                          clap_argument_t *arg,
                          bool *mutex_group_used,
                          const char *option_str,
                          clap_error_t *error) {
    (void)option_str;    
    if (arg->group_id < 0 || !mutex_group_used) {
        return true;
    }

    if (mutex_group_used[arg->group_id]) {
        const char *conflicting_opt = NULL;
        clap_mutex_group_t *group = parser->mutex_groups[arg->group_id];

        /* Find the conflicting option */
        for (size_t k = 0; k < group->arg_count; k++) {
            clap_argument_t *other = group->arguments[k];
            if (other != arg) {
                clap_buffer_t *opt_name = clap_buffer_empty();
                for (size_t m = 0; m < other->option_count; m++) {
                    if (m > 0) clap_buffer_cat(&opt_name, "/");
                    clap_buffer_cat(&opt_name, other->option_strings[m]);
                }
                conflicting_opt = clap_strdup(clap_buffer_cstr(opt_name));
                clap_buffer_free(opt_name);
                break;
            }
        }

        clap_buffer_t *current_opt = clap_buffer_empty();
        for (size_t m = 0; m < arg->option_count; m++) {
            if (m > 0) clap_buffer_cat(&current_opt, "/");
            clap_buffer_cat(&current_opt, arg->option_strings[m]);
        }

        clap_error_set(error, CLAP_ERR_MUTUALLY_EXCLUSIVE,
                       "argument %s: not allowed with argument %s",
                       clap_buffer_cstr(current_opt),
                       conflicting_opt ? conflicting_opt : "another option");

        clap_buffer_free(current_opt);
        if (conflicting_opt) {
            clap_free((void*)conflicting_opt);
        }

        return false;
    }

    mutex_group_used[arg->group_id] = true;
    return true;
}
