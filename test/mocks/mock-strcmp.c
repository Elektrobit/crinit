// SPDX-License-Identifier: MIT
/**
 * @file mock-strcmp.c
 * @brief Implementation of a mock function for strcmp().
 */
#include "mock-strcmp.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_strcmp(const char *s1, const char *s2) {
    check_expected_ptr(s1);
    check_expected_ptr(s2);
    return mock_type(int);
}
