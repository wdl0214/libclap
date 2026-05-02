// include/clap/clap_allocator.h

#ifndef CLAP_ALLOCATOR_H
#define CLAP_ALLOCATOR_H

#include <stddef.h>
#include <clap/clap_export.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Set custom memory allocator functions.
 *
 * All subsequent allocations (internal buffers, trie nodes, namespace
 * entries, etc.) use these callbacks.  Must be called before any other
 * libclap function — once parsing begins the allocator is locked in.
 *
 * @param malloc_fn  Allocation callback, must match malloc() semantics.
 *                   Must not be NULL.
 * @param free_fn    Deallocation callback, must match free() semantics.
 *                   Must not be NULL.
 * @param realloc_fn Reallocation callback, must match realloc() semantics.
 *                   Pass NULL if not needed (libclap falls back to
 *                   malloc+copy+free internally).
 *
 * @note This function must be called before any parser is created.
 *       Calling it after clap_parser_new() has undefined behavior.
 */
CLAP_EXPORT void clap_set_allocator(
    void *(*malloc_fn)(size_t),
    void (*free_fn)(void*),
    void *(*realloc_fn)(void*, size_t)
);

/* clap_malloc, clap_calloc, clap_free, clap_realloc, clap_strdup, clap_strndup
 * are internal utilities declared in clap_parser_internal.h */

#ifdef __cplusplus
}
#endif

#endif /* CLAP_ALLOCATOR_H */
