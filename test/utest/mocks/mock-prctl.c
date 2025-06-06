// SPDX-License-Identifier: MIT
/**
 * @file mock-mount.c
 * @brief Implementation of a mock function for prctl().
 */
#include "mock-prctl.h"

#include "common.h"
#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_prctl(int op, ...) {
    check_expected(op);

    return mock_type(int);
}
