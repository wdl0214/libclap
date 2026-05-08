/**
 * @file clap_color.h
 * @brief ANSI color support for libclap help/error output
 *
 * Detection follows the no-color.org convention:
 *   1. NO_COLOR env var set        → disabled
 *   2. FORCE_COLOR env var set     → enabled
 *   3. CLICOLOR=0                  → disabled
 *   4. CLICOLOR_FORCE != 0         → enabled
 *   5. isatty(stream) check        → enabled if TTY
 *
 * On Windows, ENABLE_VIRTUAL_TERMINAL_PROCESSING is requested on first use.
 */

#ifndef CLAP_COLOR_H
#define CLAP_COLOR_H

#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * ANSI style constants
 * ------------------------------------------------------------------------- */

#define CLAP_ANSI_RESET   "\033[0m"
#define CLAP_ANSI_BOLD    "\033[1m"
#define CLAP_ANSI_DIM     "\033[2m"
#define CLAP_ANSI_RED     "\033[31m"
#define CLAP_ANSI_GREEN   "\033[32m"
#define CLAP_ANSI_YELLOW  "\033[33m"
#define CLAP_ANSI_CYAN    "\033[36m"

/* -------------------------------------------------------------------------
 * Semantic color keys (matching Python 3.14 argparse theme sections)
 * ------------------------------------------------------------------------- */

typedef enum {
    CLAP_COLOR_USAGE_PROG,    /* program name in usage line      → bold     */
    CLAP_COLOR_HEADING,       /* section headers                 → bold     */
    CLAP_COLOR_OPTION_SHORT,  /* short options (-h, -v)          → green    */
    CLAP_COLOR_OPTION_LONG,   /* long options (--help, --version)→ cyan     */
    CLAP_COLOR_METAVAR,       /* metavar placeholders (FILE)     → yellow   */
    CLAP_COLOR_CHOICES,       /* choices ({a,b,c})               → yellow   */
    CLAP_COLOR_DEFAULT,       /* (default: ...) text             → dim      */
    CLAP_COLOR_SUBCOMMAND,    /* subcommand names                → cyan     */
    CLAP_COLOR_ERROR,         /* error messages                  → red      */
    CLAP_COLOR_WARNING,       /* deprecation warnings            → yellow   */
    CLAP_COLOR_COUNT
} clap_color_key_t;

/* -------------------------------------------------------------------------
 * Color theme
 * ------------------------------------------------------------------------- */

typedef struct clap_color_theme_s {
    const char *codes[CLAP_COLOR_COUNT];
    const char *reset;
    bool enabled;
} clap_color_theme_t;

/* -------------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------------- */

/**
 * @brief Initialize a color theme with the default argparse-compatible scheme.
 *
 * Call this once to populate @p theme, then optionally override individual
 * color keys before attaching the theme to a parser.
 *
 * @param theme  Pointer to an uninitialized clap_color_theme_t.
 */
void clap_color_theme_init(clap_color_theme_t *theme);

/**
 * @brief Run terminal / environment detection and set theme->enabled.
 *
 * Checks NO_COLOR, FORCE_COLOR, CLICOLOR, CLICOLOR_FORCE, and isatty().
 * On Windows, enables ENABLE_VIRTUAL_TERMINAL_PROCESSING on @p stream.
 *
 * Call this after clap_color_theme_init() and before first output, or
 * let clap_parser_set_color() call it automatically.
 *
 * @param theme   Theme to update.
 * @param stream  Stream that will receive output (stdout or stderr).
 */
void clap_color_theme_detect(clap_color_theme_t *theme, FILE *stream);

/**
 * @brief Get the visual (printable) column count of a string.
 *
 * ANSI escape sequences (ESC [ ... m) are not counted as printable
 * characters.  Use this for column alignment when a string may contain
 * embedded color codes.
 *
 * @param s  NUL-terminated string, possibly containing ANSI escapes.
 * @return Number of visible columns.
 */
size_t clap_color_visual_length(const char *s);

#ifdef __cplusplus
}
#endif

#endif /* CLAP_COLOR_H */
