// SPDX-License-Identifier: MIT
/**
 * @file mock-kill.c
 * @brief Implementation of a mock function for kill().
 */
#include "mock-kill.h"
#include "common.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_kill(pid_t pid, int sig) {
    CRINIT_PARAM_UNUSED(pid);
    CRINIT_PARAM_UNUSED(sig);

    return 0;
}
