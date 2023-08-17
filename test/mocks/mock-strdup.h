// SPDX-License-Identifier: MIT
/**
 * @file mock-strdup.h
 * @brief Header declaring a mock function for strdup().
 */
#ifndef __MOCK_STRDUP_H__
#define __MOCK_STRDUP_H__

#include <stddef.h>

/**
 * Mock function for strdup().
 *
 * Checks that the right parameter is given and returns a preset pointer.
 */
char *__wrap_strdup(const char *s);  // NOLINT(readability-identifier-naming)
                                     // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_STRDUP_H__ */
