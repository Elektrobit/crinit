// SPDX-License-Identifier: MIT
/**
 * @file mock-closedir.h
 * @brief Header declaring a mock function for closedir().
 */
#ifndef __MOCK_CLOSEDIR_H__
#define __MOCK_CLOSEDIR_H__

#include <dirent.h>
#include <sys/types.h>

/**
 * Mock function for closedir().
 *
 * Checks that the right parameters are given.
 */
void __wrap_closedir(DIR *dirp);  // NOLINT(readability-identifier-naming)
                                  // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_CLOSEDIR_H__ */
