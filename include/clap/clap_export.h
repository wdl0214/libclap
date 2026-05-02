/**
 * @file clap_export.h
 * @brief Symbol export/import macros for libclap
 *
 * Following the same approach as curl's CURL_EXTERN:
 * - When building the shared library with hidden visibility,
 *   CLAP_EXPORT marks symbols for public export.
 * - On Windows, handles dllexport/dllimport.
 * - For static builds, CLAP_EXPORT is a no-op.
 *
 * @note If you link libclap statically, you must define CLAP_STATIC
 *       before including any libclap header (e.g. -DCLAP_STATIC or
 *       #define CLAP_STATIC).  When using CMake, pass -DCLAP_STATIC
 *       via target_compile_definitions or add it to your project's
 *       compile flags.  This is required on all platforms, but most
 *       important on Windows where the absence of CLAP_STATIC would
 *       cause the compiler to emit __declspec(dllimport) decorations
 *       that are incompatible with a static library.
 */

#ifndef CLAP_EXPORT_H
#define CLAP_EXPORT_H

/* CLAP_STATIC: must be defined by consumers linking statically */
#if defined(CLAP_STATIC)
#  define CLAP_EXPORT

/* Windows: dllexport when building, dllimport when consuming */
#elif defined(_WIN32)
#  ifdef BUILDING_CLAP
#    define CLAP_EXPORT __declspec(dllexport)
#  else
#    define CLAP_EXPORT __declspec(dllimport)
#  endif

/* Linux/macOS: restore default visibility when library uses -fvisibility=hidden */
#elif defined(BUILDING_CLAP) && defined(CLAP_HIDDEN_SYMBOLS)
#  define CLAP_EXPORT __attribute__((visibility("default")))

/* Default: no decoration needed */
#else
#  define CLAP_EXPORT
#endif

/* ============================================================================
 * Deprecation annotation
 * ============================================================================ */

#if defined(__GNUC__) || defined(__clang__)
#  define CLAP_DEPRECATED(msg) __attribute__((deprecated(msg)))
#elif defined(_MSC_VER) && _MSC_VER >= 1900
#  define CLAP_DEPRECATED(msg) __declspec(deprecated(msg))
#else
#  define CLAP_DEPRECATED(msg)
#endif

#endif /* CLAP_EXPORT_H */
