/**
 * @file mock-fstatat.c
 * @brief Implementation of a mock function for fstatat().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "mock-fstatat.h"

#include <string.h>

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_fstatat(int fd, const char *path, struct stat *buf, int flag) {
    check_expected(fd);
    check_expected_ptr(path);
    check_expected_ptr(buf);
    check_expected(flag);
    memcpy(buf, mock_type(struct stat *), sizeof(*buf));
    return mock_type(int);
}
