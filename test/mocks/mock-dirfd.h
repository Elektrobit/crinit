// SPDX-License-Identifier: MIT
/**
 * @file mock-dirfd.h
 * @brief Header declaring a mock function for dirfd().
 */
#ifndef __MOCK_DIRFD_H__
#define __MOCK_DIRFD_H__

#include <dirent.h>
#include <sys/types.h>

/**
 * Mock function for dirfd().
 *
 * Checks that the right parameters are given and return a preset value.
 */
int __wrap_dirfd(DIR *dirp);  // NOLINT(readability-identifier-naming)
                              // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_DIRFD_H__ */
