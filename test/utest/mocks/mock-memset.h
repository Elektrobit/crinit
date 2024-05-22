// SPDX-License-Identifier: MIT
/**
 * @file mock-memset.h
 * @brief Header declaring a mock function for memset().
 */
#ifndef __MOCK_MEMSET_H__
#define __MOCK_MEMSET_H__

#include <stddef.h>

/**
 * Mock function for memset().
 *
 * Checks that the right parameters are given and return a preset pointer.
 */
void *__wrap_memset(void *str, int c, size_t n);  // NOLINT(readability-identifier-naming)
                                                  // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_MEMSET_H__ */
