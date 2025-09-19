// SPDX-License-Identifier: MIT
/**
 * @file mock-open.c
 * @brief Implementation of a mock function for open().
 */
#include "mock-open.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_open(const char *pathname, int flags) {
    check_expected_ptr(pathname);
    check_expected(flags);

    return mock_type(int);
}
