/**
 * @file clap_namespace.c
 * @brief Namespace implementation
 */

#include "clap_parser_internal.h"

clap_namespace_t* clap_namespace_new(void) {
    clap_namespace_t *ns = clap_calloc(1, sizeof(clap_namespace_t));
    if (ns) {
        ns->value_capacity = 8;
        ns->values = clap_calloc(ns->value_capacity, sizeof(clap_value_t*));
        if (!ns->values) {
            clap_free(ns);
            return NULL;
        }
    }
    return ns;
}

bool clap_namespace_merge(clap_namespace_t *dst, clap_namespace_t *src) {
    if (!dst || !src) return false;
    
    for (size_t i = 0; i < src->value_count; i++) {
        clap_value_t *src_val = src->values[i];
        
        /* Check if value already exists in dst */
        clap_value_t *dst_val = NULL;
        for (size_t j = 0; j < dst->value_count; j++) {
            if (strcmp(dst->values[j]->name, src_val->name) == 0) {
                dst_val = dst->values[j];
                break;
            }
        }
        
        /* Only copy if not already present */
        if (!dst_val) {
            switch (src_val->type) {
            case CLAP_VAL_STRING:
                clap_namespace_set_string(dst, src_val->name, src_val->data.str_val);
                break;
            case CLAP_VAL_INT:
                clap_namespace_set_int(dst, src_val->name, src_val->data.int_val);
                break;
            case CLAP_VAL_FLOAT:
                clap_namespace_set_float(dst, src_val->name, src_val->data.float_val);
                break;
            case CLAP_VAL_BOOL:
                clap_namespace_set_bool(dst, src_val->name, src_val->data.bool_val);
                break;
            case CLAP_VAL_ARRAY:
                for (size_t k = 0; k < src_val->data.array.count; k++) {
                    clap_namespace_append_string(dst, src_val->name, 
                                                  src_val->data.array.items[k]);
                }
                break;
            default:
                break;
            }
        }
    }
    
    return true;
}

static clap_value_t* find_value(clap_namespace_t *ns, const char *name) {
    for (size_t i = 0; i < ns->value_count; i++) {
        if (strcmp(ns->values[i]->name, name) == 0) {
            return ns->values[i];
        }
    }
    return NULL;
}

static clap_value_t* ensure_value(clap_namespace_t *ns, const char *name) {
    clap_value_t *val = find_value(ns, name);
    if (val) return val;

    if (ns->value_count >= ns->value_capacity) {
        size_t new_cap = ns->value_capacity * 2;
        clap_value_t **new_vals = clap_realloc(ns->values, new_cap * sizeof(clap_value_t*));
        if (!new_vals) return NULL;
        ns->values = new_vals;
        ns->value_capacity = new_cap;
    }

    val = clap_calloc(1, sizeof(clap_value_t));
    if (!val) return NULL;

    val->name = clap_strdup(name);
    ns->values[ns->value_count++] = val;
    return val;
}

bool clap_namespace_set_string(clap_namespace_t *ns, const char *name, const char *value) {
    clap_value_t *val = ensure_value(ns, name);
    if (!val) return false;

    if (val->type == CLAP_VAL_STRING && val->data.str_val) {
        clap_free(val->data.str_val);
    }

    val->type = CLAP_VAL_STRING;
    val->data.str_val = value ? clap_strdup(value) : NULL;
    return true;
}

bool clap_namespace_set_int(clap_namespace_t *ns, const char *name, int value) {
    clap_value_t *val = ensure_value(ns, name);
    if (!val) return false;

    val->type = CLAP_VAL_INT;
    val->data.int_val = value;
    return true;
}

bool clap_namespace_set_float(clap_namespace_t *ns, const char *name, double value) {
    clap_value_t *val = ensure_value(ns, name);
    if (!val) return false;

    val->type = CLAP_VAL_FLOAT;
    val->data.float_val = value;
    return true;
}

bool clap_namespace_set_bool(clap_namespace_t *ns, const char *name, bool value) {
    clap_value_t *val = ensure_value(ns, name);
    if (!val) return false;

    val->type = CLAP_VAL_BOOL;
    val->data.bool_val = value;
    return true;
}

bool clap_namespace_append_string(clap_namespace_t *ns, const char *name, const char *value) {
    if (!value) return true;

    clap_value_t *val = ensure_value(ns, name);
    if (!val) return false;

    if (val->type != CLAP_VAL_ARRAY) {
        val->type = CLAP_VAL_ARRAY;
        val->data.array.capacity = 4;
        val->data.array.items = clap_calloc(4, sizeof(char*));
        val->data.array.count = 0;
    }

    if (val->data.array.count >= val->data.array.capacity) {
        size_t new_cap = val->data.array.capacity * 2;
        char **new_items = clap_realloc(val->data.array.items, new_cap * sizeof(char*));
        if (!new_items) return false;
        val->data.array.items = new_items;
        val->data.array.capacity = new_cap;
    }

    val->data.array.items[val->data.array.count++] = clap_strdup(value);
    return true;
}

bool clap_namespace_get_string(clap_namespace_t *ns, const char *name, const char **value) {
    clap_value_t *val = find_value(ns, name);
    if (!val || val->type != CLAP_VAL_STRING) return false;
    *value = val->data.str_val;
    return true;
}

bool clap_namespace_get_int(clap_namespace_t *ns, const char *name, int *value) {
    clap_value_t *val = find_value(ns, name);
    if (!val || val->type != CLAP_VAL_INT) return false;
    *value = val->data.int_val;
    return true;
}

bool clap_namespace_get_float(clap_namespace_t *ns, const char *name, double *value) {
    clap_value_t *val = find_value(ns, name);
    if (!val || val->type != CLAP_VAL_FLOAT) return false;
    *value = val->data.float_val;
    return true;
}

bool clap_namespace_get_bool(clap_namespace_t *ns, const char *name, bool *value) {
    clap_value_t *val = find_value(ns, name);
    if (!val || val->type != CLAP_VAL_BOOL) return false;
    *value = val->data.bool_val;
    return true;
}

bool clap_namespace_get_string_array(clap_namespace_t *ns, const char *name, const char ***values, size_t *count) {
    clap_value_t *val = find_value(ns, name);
    if (!val || val->type != CLAP_VAL_ARRAY) return false;
    *values = (const char**)val->data.array.items;
    *count = val->data.array.count;
    return true;
}

bool clap_namespace_has(clap_namespace_t *ns, const char *name) {
    if (!ns || !name) return false;
    return find_value(ns, name) != NULL;
}

static void value_free(clap_value_t *val) {
    if (!val) return;

    clap_free(val->name);

    switch (val->type) {
    case CLAP_VAL_STRING:
        clap_free(val->data.str_val);
        break;
    case CLAP_VAL_ARRAY:
        for (size_t i = 0; i < val->data.array.count; i++) {
            clap_free(val->data.array.items[i]);
        }
        clap_free(val->data.array.items);
        break;
    default:
        break;
    }

    clap_free(val);
}

void clap_namespace_free(clap_namespace_t *ns) {
    if (!ns) return;

    for (size_t i = 0; i < ns->value_count; i++) {
        value_free(ns->values[i]);
    }

    clap_free(ns->values);
    clap_free(ns);
}
