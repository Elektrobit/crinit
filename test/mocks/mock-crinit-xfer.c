// SPDX-License-Identifier: MIT
/**
 * @file mock-crinit-xfer.c
 * @brief Implementation of a mock function for crinitXfer().
 */
#include "mock-crinit-xfer.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_crinitXfer(const char *sockFile, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd) {
    check_expected(sockFile);
    check_expected(res);
    check_expected(cmd);

    return mock_type(int);
}
