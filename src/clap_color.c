/**
 * @file clap_color.c
 * @brief ANSI color support implementation
 */

#include "clap_parser_internal.h"
#include <clap/clap_color.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
static bool win32_vt_enabled = false;
static bool win32_vt_checked = false;

static void ensure_virtual_terminal(FILE *stream) {
    if (win32_vt_checked) return;
    win32_vt_checked = true;

    DWORD handle_id = (stream == stderr) ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE;
    HANDLE h = GetStdHandle(handle_id);
    if (h == INVALID_HANDLE_VALUE) return;

    DWORD mode = 0;
    if (!GetConsoleMode(h, &mode)) return;

    if (SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
        win32_vt_enabled = true;
    }
}

static bool stream_is_tty(FILE *stream) {
    ensure_virtual_terminal(stream);
    return win32_vt_enabled;
}
#else
#include <unistd.h>

static bool stream_is_tty(FILE *stream) {
    int fd = fileno(stream);
    return fd >= 0 && isatty(fd);
}
#endif

void clap_color_theme_init(clap_color_theme_t *theme) {
    if (!theme) return;

    /* Default color scheme matching Python 3.14 argparse */
    theme->codes[CLAP_COLOR_USAGE_PROG]   = CLAP_ANSI_BOLD;
    theme->codes[CLAP_COLOR_HEADING]      = CLAP_ANSI_BOLD;
    theme->codes[CLAP_COLOR_OPTION_SHORT] = CLAP_ANSI_GREEN;
    theme->codes[CLAP_COLOR_OPTION_LONG]  = CLAP_ANSI_CYAN;
    theme->codes[CLAP_COLOR_METAVAR]      = CLAP_ANSI_YELLOW;
    theme->codes[CLAP_COLOR_CHOICES]      = CLAP_ANSI_YELLOW;
    theme->codes[CLAP_COLOR_DEFAULT]      = CLAP_ANSI_DIM;
    theme->codes[CLAP_COLOR_SUBCOMMAND]   = CLAP_ANSI_CYAN;
    theme->codes[CLAP_COLOR_ERROR]        = CLAP_ANSI_RED;
    theme->codes[CLAP_COLOR_WARNING]      = CLAP_ANSI_YELLOW;
    theme->reset = CLAP_ANSI_RESET;
    theme->enabled = false;
}

void clap_color_theme_detect(clap_color_theme_t *theme, FILE *stream) {
    if (!theme) return;

    /* 1. NO_COLOR set → disabled (https://no-color.org) */
    const char *no_color = getenv("NO_COLOR");
    if (no_color && no_color[0] != '\0') {
        theme->enabled = false;
        return;
    }

    /* 2. FORCE_COLOR set → enabled */
    const char *force_color = getenv("FORCE_COLOR");
    if (force_color && force_color[0] != '\0') {
        theme->enabled = true;
        return;
    }

    /* 3. CLICOLOR=0 → disabled */
    const char *clicolor = getenv("CLICOLOR");
    if (clicolor && clicolor[0] == '0' && clicolor[1] == '\0') {
        theme->enabled = false;
        return;
    }

    /* 4. CLICOLOR_FORCE != 0 → enabled */
    const char *clicolor_force = getenv("CLICOLOR_FORCE");
    if (clicolor_force && clicolor_force[0] != '\0'
        && !(clicolor_force[0] == '0' && clicolor_force[1] == '\0')) {
        theme->enabled = true;
        return;
    }

    /* 5. TTY check */
    theme->enabled = stream_is_tty(stream);
}

size_t clap_color_visual_length(const char *s) {
    if (!s) return 0;

    size_t len = 0;
    while (*s) {
        if (*s == '\033' && *(s + 1) == '[') {
            s += 2;
            while (*s && *s != 'm') s++;
            if (*s == 'm') s++;
            continue;
        }
        len++;
        s++;
    }
    return len;
}

/* Append literal text with optional color wrapping.
 * If theme is NULL or disabled, appends plain text.
 * Otherwise appends: code + text + reset
 */
bool clap_buffer_cat_colored(clap_buffer_t **buf, const clap_color_theme_t *theme,
                             clap_color_key_t key, const char *text) {
    if (!text || !*text) return true;
    if (!theme || !theme->enabled) return clap_buffer_cat(buf, text);

    const char *code = theme->codes[key];
    if (!code || !*code) return clap_buffer_cat(buf, text);

    return clap_buffer_cat(buf, code)
        && clap_buffer_cat(buf, text)
        && clap_buffer_cat(buf, theme->reset);
}
