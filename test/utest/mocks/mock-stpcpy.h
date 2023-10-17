// SPDX-License-Identifier: MIT
/**
 * @file mock-stpcpy.h
 * @brief Header declaring a mock function for stpcpy().
 */
#ifndef __MOCK_STPCPY_H__
#define __MOCK_STPCPY_H__

#include <string.h>

/**
 * Mock function for stpcpy().
 *
 * Checks that the right parameter is given and returns a preset pointer.
 */
char *__wrap_stpcpy(char *dest, const char *src);  // NOLINT(readability-identifier-naming)
                                                   // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_STPCPY_H__ */
