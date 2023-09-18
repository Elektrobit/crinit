// SPDX-License-Identifier: MIT
/**
 * @file mock-scandir.h
 * @brief Header declaring a mock function for scandir().
 */
#ifndef __MOCK_SCANDIR_H__
#define __MOCK_SCANDIR_H__

#include <dirent.h>
#include <sys/types.h>

/**
 * Mock function for scandir().
 *
 * Checks that the right parameters are given and return a preset value.
 */
int __wrap_scandir(const char *dirp,  // NOLINT(readability-identifier-naming)
                   struct dirent ***namelist, int (*filter)(const struct dirent *),
                   int (*compar)(const struct dirent **, const struct dirent **));
// Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_SCANDIR_H__ */
