// SPDX-License-Identifier: MIT
/**
 * @file mock-mkdirat.c
 * @brief Implementation of a mock function for mkdirat().
 */
#include "mock-mkdirat.h"

#include <fcntl.h>

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_mkdirat(int dirfd, const char *pathname, mode_t mode) {
    check_expected(dirfd);
    check_expected_ptr(pathname);
    check_expected(mode);

    return mock_type(int);
}
