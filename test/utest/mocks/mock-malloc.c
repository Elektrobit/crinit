// SPDX-License-Identifier: MIT
/**
 * @file mock-malloc.c
 * @brief Implementation of a mock function for malloc().
 */
#include "mock-malloc.h"

#include "unit_test.h"

bool crinitMockMallocEnabled = false;

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
void *__wrap_malloc(size_t size) {
    if (crinitMockMallocEnabled) {
        check_expected(size);
        return mock_ptr_type(void *);
    } else {
        return __real_malloc(size);
    }
}
