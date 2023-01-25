/**
 * @file mock-dirfd.c
 * @brief Implementation of a mock function for dirfd().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "mock-dirfd.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_dirfd(DIR *dirp) {
    check_expected_ptr(dirp);
    return mock_type(int);
}
