/**
 * @file mock-crinit-xfer.c
 * @brief Implementation of a mock function for crinitXfer().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
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
