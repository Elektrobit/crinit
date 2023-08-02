// SPDX-License-Identifier: MIT
/**
 * @file mock-free.c
 * @brief Implementation of a mock function for free().
 */
#include "mock-free.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
void __wrap_free(void *ptr) {
    check_expected_ptr(ptr);
}
