/**
 * @file mock-realloc.h
 * @brief Header declaring a mock function for realloc().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
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
