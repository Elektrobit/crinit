// SPDX-License-Identifier: MIT
/**
 * @file mock-close.h
 * @brief Header declaring a mock function for close().
 */
#ifndef __MOCK_CLOSE_H__
#define __MOCK_CLOSE_H__

/**
 * Mock function for close().
 *
 * Checks that the right parameters are given and return a preset pointer.
 */
int __wrap_close(int fd);  // NOLINT(readability-identifier-naming)
                           // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_CLOSE_H__ */
