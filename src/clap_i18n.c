/**
 * @file clap_i18n.c
 * @brief Internationalization implementation
 */

#include "clap_parser_internal.h"

static struct {
    clap_translate_func func;
    void *data;
    bool locked;
} g_translator = { NULL, NULL, false };

void clap_set_translator(clap_translate_func translator, void *user_data) {
    if (g_translator.locked) return;
    g_translator.func = translator;
    g_translator.data = user_data;
    g_translator.locked = true;
}

const char* clap_i18n_translate(const char *msgid) {
    if (g_translator.func) {
        return g_translator.func(msgid, g_translator.data);
    }
    return msgid;
}
