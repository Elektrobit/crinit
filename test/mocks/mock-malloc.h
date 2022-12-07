/**
 * @file mock-malloc.h
 * @brief Header declaring a mock function for malloc().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __MOCK_MALLOC_H__
#define __MOCK_MALLOC_H__

#include <stddef.h>

/**
 * Mock function for malloc().
 *
 * Checks that the right parameters are given and return a preset pointer.
 */
void *__wrap_malloc(size_t size);  // NOLINT(readability-identifier-naming)
                                   // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_MALLOC_H__ */
