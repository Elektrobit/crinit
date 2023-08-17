// SPDX-License-Identifier: MIT
/**
 * @file mock-opendir.h
 * @brief Header declaring a mock function for opendir().
 */
#ifndef __MOCK_OPENDIR_H__
#define __MOCK_OPENDIR_H__

#include <dirent.h>
#include <sys/types.h>

/**
 * Mock function for opendir().
 *
 * Checks that the right parameters are given and return a preset pointer.
 */
DIR *__wrap_opendir(const char *name);  // NOLINT(readability-identifier-naming)
                                        // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_OPENDIR_H__ */
