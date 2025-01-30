// SPDX-License-Identifier: MIT
/**
 * @file mock-reboot.c
 * @brief Implementation of a mock function for reboot().
 */
#include "mock-reboot.h"
#include "common.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_reboot(int cmd) {
    CRINIT_PARAM_UNUSED(cmd);

    return 0;
}
