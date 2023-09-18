// SPDX-License-Identifier: MIT
/**
 * @file mock-strcpy.h
 * @brief Header declaring a mock function for strcpy().
 */
#ifndef __MOCK_STRCPY_H__
#define __MOCK_STRCPY_H__

#include <string.h>

/**
 * Mock function for strcpy().
 *
 * Checks that the right parameter is given and returns a preset pointer.
 */
char *__wrap_strcpy(char *dest, const char *src);  // NOLINT(readability-identifier-naming)
                                                   // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_STRCPY_H__ */
