/**
 * @file mock-build-rtim-cmd.c
 * @brief Implementation of a mock function for crinitBuildRtimCmd().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "mock-build-rtim-cmd.h"

#include <stdio.h>

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_crinitBuildRtimCmd(crinitRtimCmd_t *c, crinitRtimOp_t op, int argc, ...) {
    check_expected(c);
    check_expected(op);
    check_expected(argc);

    va_list vlist;
    const char *vargs[argc];
    va_start(vlist, argc);
    for (int i = 0; i < argc; i++) {
        char param[32];
        snprintf(param, sizeof(param), "vargs[%d]", i);
        vargs[i] = va_arg(vlist, const char *);
        check_expected_dynamic(param, vargs[i]);
    }
    va_end(vlist);

    return mock_type(int);
}
