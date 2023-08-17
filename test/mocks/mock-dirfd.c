// SPDX-License-Identifier: MIT
/**
 * @file mock-dirfd.c
 * @brief Implementation of a mock function for dirfd().
 */
#include "mock-dirfd.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_dirfd(DIR *dirp) {
    check_expected_ptr(dirp);
    return mock_type(int);
}
