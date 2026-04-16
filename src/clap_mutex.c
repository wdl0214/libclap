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
    return true;
}