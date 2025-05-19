// SPDX-License-Identifier: MIT
/**
 * @file mock-mount.c
 * @brief Implementation of a mock function for cap_get_bound().
 */
#include "mock-cap_get_bound.h"

#include <sys/capability.h>

#include "logio.h"
#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_cap_get_bound(cap_value_t capVal) {
    if (capVal == CAP_KILL) {
        crinitInfoPrint("Mocking cap value %d as unsupported", capVal);
        return -1;
    }

    if (capVal > CAP_LAST_CAP) {
        crinitInfoPrint("Exceeding last supported capability: %d", capVal);
        return -1;
    }

    return 0;
}
