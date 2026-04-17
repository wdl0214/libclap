// include/clap/clap_allocator.h

#ifndef CLAP_ALLOCATOR_H
#define CLAP_ALLOCATOR_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Set custom memory allocator functions
 */
void clap_set_allocator(
    void *(*malloc_fn)(size_t),
    void (*free_fn)(void*),
    void *(*realloc_fn)(void*, size_t)
);

/**
 * @brief Allocate zero-initialized memory
 */
void* clap_malloc(size_t size);

/**
 * @brief Allocate zero-initialized array
 */
void* clap_calloc(size_t nmemb, size_t size);

/**
 * @brief Free allocated memory
 */
void clap_free(void *ptr);

/**
 * @brief Reallocate memory
 */
void* clap_realloc(void *ptr, size_t size);

/**
 * @brief Duplicate a string
 */
char* clap_strdup(const char *str);

/**
 * @brief Duplicate at most n characters of a string
 */
char* clap_strndup(const char *str, size_t n);

#ifdef __cplusplus
}
#endif

#endif /* CLAP_ALLOCATOR_H */
