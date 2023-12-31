// SPDX-License-Identifier: MIT
/**
 * @file mock-strlen.c
 * @brief Implementation of a mock function for strlen().
 */
#include "mock-strlen.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
size_t __wrap_strlen(const char *s) {
    check_expected_ptr(s);
    return mock_type(size_t);
}
