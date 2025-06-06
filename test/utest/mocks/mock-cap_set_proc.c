// SPDX-License-Identifier: MIT
/**
 * @file mock-cap_set_proc.c
 * @brief Implementation of a mock function for cap_set_proc().
 */
#include "mock-cap_set_proc.h"

#include "common.h"
#include "logio.h"
#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_cap_set_proc(cap_t cap) {
    CRINIT_PARAM_UNUSED(cap);

    return 0;
}
