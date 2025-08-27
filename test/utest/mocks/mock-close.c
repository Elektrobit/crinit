// SPDX-License-Identifier: MIT
/**
 * @file mock-close.c
 * @brief Implementation of a mock function for close().
 */
#include "mock-close.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_close(int fd) {
    check_expected(fd);

    return mock_type(int);
}
