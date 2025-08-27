// SPDX-License-Identifier: MIT
/**
 * @file mock-openat.h
 * @brief Header declaring a mock function for openat().
 */
#ifndef __MOCK_OPENAT_H__
#define __MOCK_OPENAT_H__

/**
 * Mock function for openat().
 *
 * Checks that the right parameters are given and return a preset pointer.
 */
int __wrap_openat(int dirfd, const char *pathname,  // NOLINT(readability-identifier-naming)
                  int flags);
// Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_OPENAT_H__ */
