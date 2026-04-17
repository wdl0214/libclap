/**
* @file clap.c
 * @brief Library initialization and version
 */

#include <clap/clap.h>

#define CLAP_VERSION_MAJOR 1
#define CLAP_VERSION_MINOR 0
#define CLAP_VERSION_PATCH 0

const char* clap_version(void) {
    static char version[32];
    snprintf(version, sizeof(version), "%d.%d.%d",
             CLAP_VERSION_MAJOR, CLAP_VERSION_MINOR, CLAP_VERSION_PATCH);
    return version;
}
