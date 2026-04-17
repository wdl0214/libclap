/**
 * @file clap_arena.c
 * @brief Arena allocator implementation
 */

#include "clap_parser_internal.h"

clap_arena_t* clap_arena_new(size_t chunk_size) {
    if (chunk_size < 4096) chunk_size = 4096;

    clap_arena_t *arena = clap_calloc(1, sizeof(clap_arena_t));
    if (!arena) return NULL;

    arena->chunk_size = chunk_size;

    clap_arena_chunk_t *chunk = clap_malloc(sizeof(clap_arena_chunk_t) + chunk_size);
    if (!chunk) {
        clap_free(arena);
        return NULL;
    }

    chunk->next = NULL;
    chunk->capacity = chunk_size;
    chunk->used = 0;

    arena->current = chunk;
    arena->first = chunk;

    return arena;
}

void clap_arena_free(clap_arena_t *arena) {
    if (!arena) return;

    clap_arena_chunk_t *chunk = arena->first;
    while (chunk) {
        clap_arena_chunk_t *next = chunk->next;
        clap_free(chunk);
        chunk = next;
    }

    clap_free(arena);
}

void* clap_arena_alloc(clap_arena_t *arena, size_t size) {
    if (!arena || size == 0) return NULL;

    /* Align size to pointer boundary */
    size = (size + sizeof(void*) - 1) & ~(sizeof(void*) - 1);

    if (arena->current->used + size > arena->current->capacity) {
        size_t new_size = arena->chunk_size;
        if (size > new_size) new_size = size;

        clap_arena_chunk_t *new_chunk = clap_malloc(sizeof(clap_arena_chunk_t) + new_size);
        if (!new_chunk) return NULL;

        new_chunk->next = NULL;
        new_chunk->capacity = new_size;
        new_chunk->used = 0;

        arena->current->next = new_chunk;
        arena->current = new_chunk;
    }

    void *ptr = (char*)arena->current->data + arena->current->used;
    arena->current->used += size;

    memset(ptr, 0, size);
    return ptr;
}

char* clap_arena_strdup(clap_arena_t *arena, const char *str) {
    if (!arena || !str) return NULL;

    size_t len = strlen(str) + 1;
    char *copy = clap_arena_alloc(arena, len);
    if (copy) memcpy(copy, str, len);
    return copy;
}

void clap_arena_reset(clap_arena_t *arena) {
    if (!arena) return;

    clap_arena_chunk_t *chunk = arena->first->next;
    while (chunk) {
        clap_arena_chunk_t *next = chunk->next;
        clap_free(chunk);
        chunk = next;
    }

    arena->first->next = NULL;
    arena->first->used = 0;
    arena->current = arena->first;
}
