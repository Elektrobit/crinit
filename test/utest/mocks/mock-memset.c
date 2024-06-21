// SPDX-License-Identifier: MIT
/**
 * @file mock-memset.c
 * @brief Implementation of a mock function for memset().
 */
#include "mock-memset.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
void *__wrap_memset(void *str, int c, size_t n) {
    check_expected_ptr(str);
    check_expected(c);
    check_expected(n);
    return mock_ptr_type(void *);
}
