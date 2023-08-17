// SPDX-License-Identifier: MIT
/**
 * @file mock-envset-destroy.c
 * @brief Implementation of a mock function for crinitEnvSetDestroy().
 */
#include "mock-envset-destroy.h"

#include "envset.h"
#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_crinitEnvSetDestroy(crinitEnvSet_t *es) {
    check_expected_ptr(es);
    return mock_type(int);
}
