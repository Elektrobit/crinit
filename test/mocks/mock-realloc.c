// SPDX-License-Identifier: MIT
/**
 * @file mock-realloc.c
 * @brief Implementation of a mock function for malloc().
 */
#include "mock-realloc.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
void *__wrap_realloc(void *ptr, size_t size) {
    check_expected(ptr);
    check_expected(size);
    return mock_ptr_type(void *);
}
