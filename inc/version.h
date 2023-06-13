/**
 * @file version.h
 * @brief Header definitions related to component versioning.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __VERSION_H__
#define __VERSION_H__

#include <stdint.h>

/**
 * Maximum length of a git commit ID in characters.
 */
#define CRINIT_GIT_HASH_MAX_LEN 40

/**
 * Structure to hold version information for this software component.
 */
typedef struct crinitVersion_t {
    uint8_t major;                        ///< Major version.
    uint8_t minor;                        ///< Minor version.
    uint8_t micro;                        ///< Micro/patch version.
    char git[CRINIT_GIT_HASH_MAX_LEN + 1];  ///< git commit Hash
} crinitVersion_t;

/**
 * Crinit software component version. Generated via CMake in version.c.in.
 */
extern const crinitVersion_t crinitVersion;

/**
 * Returns a const char pointer to a formatted human-readable version string for this software component.
 *
 * Generated via CMake in version.c.in. Format is `MAJOR.MINOR.MICRO.GIT` if a git commit hash exists or
 * `MAJOR.MINOR.MICRO` if the git hash is an empty string.
 *
 * @return  Pointer to a version string constant.
 */
const char *crinitGetVersionString(void);

#endif /* __VERSION_H__ */
