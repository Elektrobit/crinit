// SPDX-License-Identifier: MIT
/**
 * @file mock-strdup.c
 * @brief Implementation of a mock function for strdup().
 */
#include "mock-strdup.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
char *__wrap_strdup(const char *s) {
    check_expected_ptr(s);
    return mock_ptr_type(char *);
}
