// SPDX-License-Identifier: MIT
/**
 * @file mock-free.h
 * @brief Header declaring a mock function for free().
 */
#ifndef __MOCK_FREE_H__
#define __MOCK_FREE_H__

/**
 * Mock function for free().
 *
 * Checks that the right parameters are given and return a preset pointer.
 */
void __wrap_free(void *ptr);  // NOLINT(readability-identifier-naming)
                              // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_FREE_H__ */
