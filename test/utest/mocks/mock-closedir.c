// SPDX-License-Identifier: MIT
/**
 * @file mock-closedir.c
 * @brief Implementation of a mock function for closedir().
 */
#include "mock-closedir.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
void __wrap_closedir(DIR *dirp) {
    check_expected_ptr(dirp);
}
