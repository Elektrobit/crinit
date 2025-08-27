// SPDX-License-Identifier: MIT
/**
 * @file mock-openat.c
 * @brief Implementation of a mock function for openat().
 */
#include "mock-openat.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_openat(int dirfd, const char *pathname, int flags) {
    check_expected(dirfd);
    check_expected_ptr(pathname);
    check_expected(flags);

    return mock_type(int);
}
