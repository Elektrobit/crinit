// SPDX-License-Identifier: MIT
/**
 * @file mock-calloc.h
 * @brief Header declaring a mock function for calloc().
 */
#ifndef __MOCK_CALLOC_H__
#define __MOCK_CALLOC_H__

#include <stddef.h>

/**
 * Mock function for calloc().
 *
 * Checks that the right parameters are given and return a preset pointer.
 */
void *__wrap_calloc(size_t num, size_t size);  // NOLINT(readability-identifier-naming)
                                               // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_CALLOC_H__ */
