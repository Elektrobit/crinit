// SPDX-License-Identifier: MIT
/**
 * @file case-invalid-capability.c
 * @brief Unit test for crinitCapSetAmbient(), failing execution, invalid capabilities.
 */

#include <sys/prctl.h>

#include "capabilities.h"
#include "common.h"
#include "logio.h"
#include "unit_test.h"
#include "utest-crinit-cap-set-ambient.h"

#ifndef ENABLE_CAPABILITIES
#define ENABLE_CAPABILITIES
#endif

void test_crinitCapSetAmbient_invalid_capability_typo(void **state) {
    CRINIT_PARAM_UNUSED(state);
    uint64_t capBitmask = 0;

    assert_int_equal(
        crinitCapConvertToBitmask(&capBitmask, "CAP_SETPCAP CAP_KILL CAP_FSETID CAP_DAC_OVERRIDE CAP_CHOWN"), 0);
    crinitInfoPrint("Test capability bitmask %#lx", capBitmask);
    expect_value_count(__wrap_prctl, op, PR_CAP_AMBIENT, 3);
    will_return_count(__wrap_prctl, 0, 3);
    assert_int_equal(crinitCapSetAmbient(capBitmask), -1);
}

void test_crinitCapSetAmbient_invalid_capability_range(void **state) {
    CRINIT_PARAM_UNUSED(state);
    assert_int_equal(crinitCapSetAmbient(1uLL << (CAP_LAST_CAP + 1)), -1);
}
