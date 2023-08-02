// SPDX-License-Identifier: MIT
/**
 * @file mock-opendir.c
 * @brief Implementation of a mock function for opendir().
 */
#include "mock-opendir.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
DIR *__wrap_opendir(const char *name) {
    check_expected_ptr(name);
    return mock_ptr_type(DIR *);
}
