// SPDX-License-Identifier: MIT
/**
 * @file mock-umount2.c
 * @brief Implementation of a mock function for umount2().
 */
#include "mock-umount2.h"

#include "common.h"
#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_umount2(const char *target, int flags) {
    CRINIT_PARAM_UNUSED(target);
    CRINIT_PARAM_UNUSED(flags);

    return 0;
}
