/**
 * @file clap_trie.c
 * @brief Trie implementation for fast option lookup
 */

#include "clap_parser_internal.h"

clap_trie_t* clap_trie_new(void) {
    clap_trie_t *trie = clap_calloc(1, sizeof(clap_trie_t));
    if (!trie) return NULL;

    trie->arena = clap_arena_new(65536);
    if (!trie->arena) {
        clap_free(trie);
        return NULL;
    }

    trie->root = clap_arena_alloc(trie->arena, sizeof(clap_trie_node_t));
    if (!trie->root) {
        clap_arena_free(trie->arena);
        clap_free(trie);
        return NULL;
    }

    return trie;
}

void clap_trie_free(clap_trie_t *trie) {
    if (!trie) return;
    clap_arena_free(trie->arena);
    clap_free(trie);
}

bool clap_trie_insert(clap_trie_t *trie, const char *key, clap_argument_t *arg) {
    if (!trie || !key || !arg) return false;

    clap_trie_node_t *current = trie->root;

    for (const char *p = key; *p; p++) {
        unsigned char idx = (unsigned char)*p;

        if (!current->children[idx]) {
            current->children[idx] = clap_arena_alloc(trie->arena, sizeof(clap_trie_node_t));
            if (!current->children[idx]) return false;
            current->children[idx]->character = *p;
            trie->node_count++;
        }

        current = current->children[idx];
    }

    if (current->is_end_of_word) return false;

    current->is_end_of_word = true;
    current->argument = arg;
    return true;
}

clap_argument_t* clap_trie_find_exact(clap_trie_t *trie, const char *key) {
    if (!trie || !key) return NULL;

    clap_trie_node_t *current = trie->root;

    for (const char *p = key; *p; p++) {
        unsigned char idx = (unsigned char)*p;
        if (!current->children[idx]) return NULL;
        current = current->children[idx];
    }

    return current->is_end_of_word ? current->argument : NULL;
}

clap_argument_t* clap_trie_find_prefix(clap_trie_t *trie, const char *prefix, bool allow_ambiguous) {
    (void)allow_ambiguous;

    if (!trie || !prefix) return NULL;

    clap_trie_node_t *current = trie->root;

    for (const char *p = prefix; *p; p++) {
        unsigned char idx = (unsigned char)*p;
        if (!current->children[idx]) return NULL;
        current = current->children[idx];
    }

    if (current->is_end_of_word) {
        return current->argument;
    }

    /* Find first matching child */
    for (int i = 0; i < 256; i++) {
        if (current->children[i]) {
            clap_trie_node_t *child = current->children[i];
            while (child && !child->is_end_of_word) {
                int j;
                for (j = 0; j < 256; j++) {
                    if (child->children[j]) break;
                }
                if (j < 256) {
                    child = child->children[j];
                } else {
                    child = NULL;
                }
            }
            if (child && child->is_end_of_word) {
                return child->argument;
            }
        }
    }

    return NULL;
}