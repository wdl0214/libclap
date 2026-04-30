/**
 * @file clap_version.h
 * @brief Version information for libclap
 */

#ifndef CLAP_VERSION_H
#define CLAP_VERSION_H

#include <clap/clap_export.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CLAP_VERSION_MAJOR 1
#define CLAP_VERSION_MINOR 0
#define CLAP_VERSION_PATCH 0

#define CLAP_VERSION_STRING "1.0.0"

/**
 * @brief Get libclap version string
 * @return Version string in format "major.minor.patch"
 */
CLAP_EXPORT const char* clap_version(void);

/* clap_version_components was removed (declared but never implemented) */

#ifdef __cplusplus
}
#endif

#endif /* CLAP_VERSION_H */
