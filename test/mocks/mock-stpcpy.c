// SPDX-License-Identifier: MIT
/**
 * @file mock-stpcpy.c
 * @brief Implementation of a mock function for stpcpy().
 */
#include "mock-stpcpy.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
char *__wrap_stpcpy(char *dest, const char *src) {
    check_expected_ptr(dest);
    check_expected_ptr(src);
    return mock_ptr_type(char *);
}
