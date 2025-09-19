// SPDX-License-Identifier: MIT
/**
 * @file mock-open.h
 * @brief Header declaring a mock function for open().
 */
#ifndef __MOCK_OPEN_H__
#define __MOCK_OPEN_H__

/**
 * Mock function for open().
 *
 * Checks that the right parameters are given and return a preset pointer.
 */
int __wrap_open(const char *pathname, int flags);  // NOLINT(readability-identifier-naming)
                                                   // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_OPEN_H__ */
