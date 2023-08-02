// SPDX-License-Identifier: MIT
/**
 * @file mock-realloc.h
 * @brief Header declaring a mock function for realloc().
 */
#ifndef __MOCK_REALLOC_H__
#define __MOCK_REALLOC_H__

#include <stddef.h>

/**
 * Mock function for realloc().
 *
 * Checks that the right parameters are given and return a preset pointer.
 */
void *__wrap_realloc(void *ptr, size_t size);  // NOLINT(readability-identifier-naming)
                                               // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_REALLOC_H__ */
