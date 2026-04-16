/**
 * @file clap_version.h
 * @brief Version information for libclap
 */

#ifndef CLAP_VERSION_H
#define CLAP_VERSION_H

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
const char* clap_version(void);

/**
 * @brief Get libclap version as components
 * @param major Output for major version
 * @param minor Output for minor version
 * @param patch Output for patch version
 */
void clap_version_components(int *major, int *minor, int *patch);

#ifdef __cplusplus
}
#endif

#endif /* CLAP_VERSION_H */