/**
 * @file clap_display_group.c
 * @brief Argument group implementation
 */

#include "clap_parser_internal.h"

int clap_add_argument_group(clap_parser_t *parser, const char *title, const char *description) {
    if (!parser || !title) return -1;

    clap_display_group_t *group = clap_calloc(1, sizeof(clap_display_group_t));
    if (!group) return -1;

    group->id = parser->next_display_group_id++;
    group->title = clap_strdup(title);
    group->description = description ? clap_strdup(description) : NULL;
    group->arg_capacity = 4;
    group->arguments = clap_calloc(group->arg_capacity, sizeof(clap_argument_t*));

    if (!group->arguments) {
        clap_free(group->title);
        clap_free(group->description);
        clap_free(group);
        return -1;
    }

    clap_display_group_t **new_groups = clap_realloc(
        parser->display_groups,
        (parser->display_group_count + 1) * sizeof(clap_display_group_t*)
    );

    if (!new_groups) {
        clap_free(group->arguments);
        clap_free(group->title);
        clap_free(group->description);
        clap_free(group);
        return -1;
    }

    parser->display_groups = new_groups;
    parser->display_groups[parser->display_group_count++] = group;

    return group->id;
}

bool clap_argument_group_add_argument(clap_parser_t *parser, int display_group_id, clap_argument_t *arg) {
    if (!parser || display_group_id < 0 || !arg) return false;

    clap_display_group_t *group = NULL;
    for (size_t i = 0; i < parser->display_group_count; i++) {
        if (parser->display_groups[i]->id == display_group_id) {
            group = parser->display_groups[i];
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
    arg->display_group_id = display_group_id;
    return true;
}
