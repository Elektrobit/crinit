// SPDX-License-Identifier: MIT
/**
 * @file mock-strcmp.h
 * @brief Header declaring a mock function for strcmp().
 */
#ifndef __MOCK_STRCMP_H__
#define __MOCK_STRCMP_H__

#include <string.h>

/**
 * Mock function for strcmp().
 *
 * Checks that the right parameter is given and returns a preset value.
 */
int __wrap_strcmp(const char *s1, const char *s2);  // NOLINT(readability-identifier-naming)
                                                    // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_STRCMP_H__ */
