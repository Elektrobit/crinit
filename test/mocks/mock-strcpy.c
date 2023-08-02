// SPDX-License-Identifier: MIT
/**
 * @file mock-strcpy.c
 * @brief Implementation of a mock function for strcpy().
 */
#include "mock-strcpy.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
char *__wrap_strcpy(char *dest, const char *src) {
    check_expected_ptr(dest);
    check_expected_ptr(src);
    return mock_ptr_type(char *);
}
