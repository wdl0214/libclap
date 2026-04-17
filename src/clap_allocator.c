/**
 * @file clap_allocator.c
 * @brief Memory allocation interface implementation
 */

#include <clap/clap_allocator.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static void* default_malloc(size_t size) {
    return malloc(size);
}

static void default_free(void *ptr) {
    free(ptr);
}

static void* default_realloc(void *ptr, size_t size) {
    return realloc(ptr, size);
}

static struct {
    void* (*malloc_fn)(size_t);
    void (*free_fn)(void*);
    void* (*realloc_fn)(void*, size_t);
    bool locked;
} g_allocator = {
    default_malloc,
    default_free,
    default_realloc,
    false
};

void clap_set_allocator(void* (*malloc_fn)(size_t),
                        void (*free_fn)(void*),
                        void* (*realloc_fn)(void*, size_t)) {
    if (g_allocator.locked) return;

    g_allocator.malloc_fn = malloc_fn ? malloc_fn : default_malloc;
    g_allocator.free_fn = free_fn ? free_fn : default_free;
    g_allocator.realloc_fn = realloc_fn ? realloc_fn : default_realloc;
    g_allocator.locked = true;
}

void* clap_malloc(size_t size) {
    g_allocator.locked = true;
    void *ptr = g_allocator.malloc_fn(size);
    if (ptr) memset(ptr, 0, size);
    return ptr;
}

void* clap_calloc(size_t nmemb, size_t size) {
    g_allocator.locked = true;
    if (nmemb == 0 || size == 0) return NULL;

    /* Check for overflow */
    if (nmemb > 0 && size > SIZE_MAX / nmemb) return NULL;

    size_t total = nmemb * size;
    void *ptr = g_allocator.malloc_fn(total);
    if (ptr) memset(ptr, 0, total);
    return ptr;
}

void clap_free(void *ptr) {
    if (ptr) g_allocator.free_fn(ptr);
}

void* clap_realloc(void *ptr, size_t size) {
    g_allocator.locked = true;
    if (size == 0) {
        g_allocator.free_fn(ptr);
        return NULL;
    }
    return g_allocator.realloc_fn(ptr, size);
}

char* clap_strdup(const char *str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char *copy = clap_malloc(len + 1);
    if (copy) memcpy(copy, str, len + 1);
    return copy;
}

char* clap_strndup(const char *str, size_t n) {
    if (!str) return NULL;
    size_t len = strnlen(str, n);
    char *copy = clap_malloc(len + 1);
    if (copy) {
        memcpy(copy, str, len);
        copy[len] = '\0';
    }
    return copy;
}
