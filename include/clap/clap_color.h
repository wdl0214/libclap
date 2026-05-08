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

/**
 * @def CLAP_ANSI_RESET
 * @brief ANSI escape code: reset all attributes.
 */
#define CLAP_ANSI_RESET   "\033[0m"
/**
 * @def CLAP_ANSI_BOLD
 * @brief ANSI escape code: bold / increased intensity.
 */
#define CLAP_ANSI_BOLD    "\033[1m"
/**
 * @def CLAP_ANSI_DIM
 * @brief ANSI escape code: dim / decreased intensity.
 */
#define CLAP_ANSI_DIM     "\033[2m"
/**
 * @def CLAP_ANSI_RED
 * @brief ANSI escape code: red foreground.
 */
#define CLAP_ANSI_RED     "\033[31m"
/**
 * @def CLAP_ANSI_GREEN
 * @brief ANSI escape code: green foreground.
 */
#define CLAP_ANSI_GREEN   "\033[32m"
/**
 * @def CLAP_ANSI_YELLOW
 * @brief ANSI escape code: yellow foreground.
 */
#define CLAP_ANSI_YELLOW  "\033[33m"
/**
 * @def CLAP_ANSI_CYAN
 * @brief ANSI escape code: cyan foreground.
 */
#define CLAP_ANSI_CYAN    "\033[36m"

/**
 * @enum clap_color_key_t
 * @brief Semantic color keys identifying which element to color.
 *
 * Each key maps to an ANSI escape sequence in clap_color_theme_t::codes.
 * The default assignments follow Python 3.14 argparse:
 *
 * | Key                      | Default style | Applied to                          |
 * |--------------------------|---------------|-------------------------------------|
 * | CLAP_COLOR_USAGE_PROG    | bold          | Program name in usage line          |
 * | CLAP_COLOR_HEADING       | bold          | Section headers                     |
 * | CLAP_COLOR_OPTION_SHORT  | green         | Short options (-h, -v)              |
 * | CLAP_COLOR_OPTION_LONG   | cyan          | Long options (--help, --verbose)    |
 * | CLAP_COLOR_METAVAR       | yellow        | Metavar placeholders (FILE)         |
 * | CLAP_COLOR_CHOICES       | yellow        | Choices ({red,green,blue})          |
 * | CLAP_COLOR_DEFAULT       | dim           | Default value text (default: 5)     |
 * | CLAP_COLOR_SUBCOMMAND    | cyan          | Subcommand names (commit, push)     |
 * | CLAP_COLOR_ERROR         | red           | Error messages                      |
 * | CLAP_COLOR_WARNING       | yellow        | Deprecation warnings                |
 */
typedef enum {
    CLAP_COLOR_USAGE_PROG,    /**< Program name in usage line. */
    CLAP_COLOR_HEADING,       /**< Section heading text. */
    CLAP_COLOR_OPTION_SHORT,  /**< Short option flags (-h, -v). */
    CLAP_COLOR_OPTION_LONG,   /**< Long option flags (--help, --verbose). */
    CLAP_COLOR_METAVAR,       /**< Metavar placeholders (FILE, DIR). */
    CLAP_COLOR_CHOICES,       /**< Choice values ({a,b,c}). */
    CLAP_COLOR_DEFAULT,       /**< Default value text in help. */
    CLAP_COLOR_SUBCOMMAND,    /**< Subcommand names. */
    CLAP_COLOR_ERROR,         /**< Error messages. */
    CLAP_COLOR_WARNING,       /**< Deprecation warning messages. */
    CLAP_COLOR_COUNT          /**< Number of color keys (internal). */
} clap_color_key_t;

/**
 * @struct clap_color_theme_s
 * @brief Storage for ANSI color codes and enable state.
 *
 * A theme maps each clap_color_key_t to an ANSI escape sequence.
 * When @p enabled is false, all clap_buffer_cat_colored() calls
 * emit plain text regardless of the stored codes.
 *
 * Initialize with clap_color_theme_init(), then optionally override
 * individual @p codes entries with custom ANSI sequences before
 * assigning to a parser via clap_parser_set_color().
 *
 * @see clap_color_theme_init(), clap_color_theme_detect()
 */
typedef struct clap_color_theme_s {
    /**
     * @brief ANSI escape sequences for each semantic color key.
     *
     * Index by clap_color_key_t.  Must not be NULL — use empty
     * string ("") to suppress coloring for a specific key.
     */
    const char *codes[CLAP_COLOR_COUNT];
    /**
     * @brief ANSI reset sequence appended after every colored span.
     *
     * Defaults to CLAP_ANSI_RESET.  Change this if your terminal
     * uses a different reset convention.
     */
    const char *reset;
    /**
     * @brief Whether color output is active.
     *
     * Set by clap_color_theme_detect() or clap_parser_set_color().
     * When false, all coloring helpers emit plain text.
     */
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
