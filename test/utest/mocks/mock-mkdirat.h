// SPDX-License-Identifier: MIT
/**
 * @file mock-mkdirat.h
 * @brief Header declaring a mock function for mkdirat().
 */
#ifndef __MOCK_MKDIRAT_H__
#define __MOCK_MKDIRAT_H__

#include <sys/stat.h>

/**
 * Mock function for mkdirat().
 *
 * Checks that the right parameters are given and return a preset pointer.
 */
int __wrap_mkdirat(int dirfd, const char *pathname,  // NOLINT(readability-identifier-naming)
                   mode_t mode);
// Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_MKDIRAT_H__ */
