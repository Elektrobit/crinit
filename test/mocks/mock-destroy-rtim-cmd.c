// SPDX-License-Identifier: MIT
/**
 * @file mock-destroy-rtim-cmd.c
 * @brief Implementation of a mock function for crinitGlobOptSet().
 */
#include "mock-destroy-rtim-cmd.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_crinitDestroyRtimCmd(crinitRtimCmd_t *c) {
    check_expected(c);

    return mock_type(int);
}
