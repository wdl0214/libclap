// include/clap/clap_allocator.h

#ifndef CLAP_ALLOCATOR_H
#define CLAP_ALLOCATOR_H

#include <stddef.h>
#include <clap/clap_export.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Set custom memory allocator functions
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
