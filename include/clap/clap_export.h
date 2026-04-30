/**
 * @file clap_export.h
 * @brief Symbol export/import macros for libclap
 *
 * Following the same approach as curl's CURL_EXTERN:
 * - When building the shared library with hidden visibility,
 *   CLAP_EXPORT marks symbols for public export.
 * - On Windows, handles dllexport/dllimport.
 * - For static builds, CLAP_EXPORT is a no-op.
 */

#ifndef CLAP_EXPORT_H
#define CLAP_EXPORT_H

/* CLAP_STATIC: defined by users who link statically */
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

#endif /* CLAP_EXPORT_H */
