/**
 * @file clap_buffer.c
 * @brief Safe string buffer implementation
 */

#include "clap_parser_internal.h"
#include <stdarg.h>
#include <ctype.h>

#define BUFFER_INIT_SIZE 64

clap_buffer_t* clap_buffer_new(const char *init) {
    size_t len = init ? strlen(init) : 0;
    return clap_buffer_new_len(init, len);
}

clap_buffer_t* clap_buffer_new_len(const void *init, size_t len) {
    clap_buffer_t *buf = clap_malloc(sizeof(clap_buffer_t) + len + 1);
    if (!buf) return NULL;

    buf->len = len;
    buf->alloc = len + 1;

    if (init && len > 0) {
        memcpy(buf->data, init, len);
    }
    buf->data[len] = '\0';

    return buf;
}

clap_buffer_t* clap_buffer_empty(void) {
    return clap_buffer_new_len(NULL, 0);
}

void clap_buffer_free(clap_buffer_t *buf) {
    clap_free(buf);
}

static bool clap_buffer_grow(clap_buffer_t **buf, size_t extra) {
    clap_buffer_t *b = *buf;
    size_t new_len = b->len + extra;

    if (new_len + 1 <= b->alloc) return true;

    size_t new_alloc = b->alloc * 2;
    if (new_alloc < new_len + 1) new_alloc = new_len + 1;

    clap_buffer_t *new_buf = clap_realloc(b, sizeof(clap_buffer_t) + new_alloc);
    if (!new_buf) return false;

    new_buf->alloc = new_alloc;
    *buf = new_buf;
    return true;
}

bool clap_buffer_cat(clap_buffer_t **buf, const char *str) {
    if (!str) return true;
    return clap_buffer_cat_len(buf, str, strlen(str));
}

bool clap_buffer_cat_len(clap_buffer_t **buf, const void *data, size_t len) {
    if (!data || len == 0) return true;
    if (!clap_buffer_grow(buf, len)) return false;

    clap_buffer_t *b = *buf;
    memcpy(b->data + b->len, data, len);
    b->len += len;
    b->data[b->len] = '\0';

    return true;
}

bool clap_buffer_cat_printf(clap_buffer_t **buf, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    /* Determine needed size */
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int needed = vsnprintf(NULL, 0, fmt, ap_copy);
    va_end(ap_copy);

    if (needed < 0) {
        va_end(ap);
        return false;
    }

    if (!clap_buffer_grow(buf, (size_t)needed)) {
        va_end(ap);
        return false;
    }

    clap_buffer_t *b = *buf;
    vsnprintf(b->data + b->len, b->alloc - b->len, fmt, ap);
    b->len += (size_t)needed;

    va_end(ap);
    return true;
}

bool clap_buffer_copy(clap_buffer_t **buf, const char *str) {
    clap_buffer_t *b = *buf;
    b->len = 0;
    b->data[0] = '\0';
    return clap_buffer_cat(buf, str);
}

const char* clap_buffer_cstr(const clap_buffer_t *buf) {
    return buf ? buf->data : "";
}

size_t clap_buffer_len(const clap_buffer_t *buf) {
    return buf ? buf->len : 0;
}

void clap_buffer_truncate(clap_buffer_t *buf, size_t len) {
    if (!buf || len >= buf->len) return;
    buf->len = len;
    buf->data[len] = '\0';
}

void clap_buffer_sanitize(clap_buffer_t *buf) {
    if (!buf) return;

    for (size_t i = 0; i < buf->len; i++) {
        unsigned char c = (unsigned char)buf->data[i];
        if (c < 0x20 && c != '\t' && c != '\n' && c != '\r') {
            buf->data[i] = '?';
        }
    }
}