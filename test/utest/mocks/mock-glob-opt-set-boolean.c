// SPDX-License-Identifier: MIT
/**
 * @file mock-glob-opt-set-boolean.c
 * @brief Implementation of a mock function for crinitGlobOptSetBoolean().
 */
#include "mock-glob-opt-set-boolean.h"

#include "unit_test.h"

int __wrap_crinitGlobOptSetBoolean(size_t memberOffset, bool val) {  // NOLINT(readability-identifier-naming)
                                                                     // Rationale: Naming scheme fixed due to linker
                                                                     // wrapping.
    check_expected(memberOffset);
    check_expected(val);
    return mock_type(int);
}
