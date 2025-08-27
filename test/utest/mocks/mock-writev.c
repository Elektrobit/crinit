// SPDX-License-Identifier: MIT
/**
 * @file mock-writev.c
 * @brief Implementation of a mock function for writev().
 */
#include "mock-writev.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
ssize_t __wrap_writev(int fd, const struct iovec *iov, int iovcnt) {
    check_expected(fd);
    check_expected_ptr(iov);
    check_expected(iovcnt);

    return mock_type(ssize_t);
}
