// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCapSetAmbient(), successful execution.
 */

#include <sys/prctl.h>

#include "capabilities.h"
#include "common.h"
#include "logio.h"
#include "unit_test.h"
#include "utest-crinit-cap-set-ambient.h"

void test_crinitCapSetAmbient_single_capability_set(void **state) {
    CRINIT_PARAM_UNUSED(state);
    uint64_t capBitmask = 0;

    assert_int_equal(crinitCapConvertToBitmask(&capBitmask, "CAP_CHOWN"), 0);
    crinitInfoPrint("Test capability bitmask %#lx", capBitmask);
    expect_value(__wrap_prctl, op, PR_CAP_AMBIENT);
    will_return(__wrap_prctl, 0);
    assert_int_equal(crinitCapSetAmbient(capBitmask), 0);
}

void test_crinitCapSetAmbient_multiple_capability_set(void **state) {
    CRINIT_PARAM_UNUSED(state);
    uint64_t capBitmask = 0;

    assert_int_equal(crinitCapConvertToBitmask(&capBitmask, "CAP_SETPCAP CAP_DAC_OVERRIDE CAP_CHOWN"), 0);
    crinitInfoPrint("Test capability bitmask %#lx", capBitmask);
    expect_value_count(__wrap_prctl, op, PR_CAP_AMBIENT, 3);
    will_return_count(__wrap_prctl, 0, 3);
    assert_int_equal(crinitCapSetAmbient(capBitmask), 0);
}

void test_crinitCapSetAmbient_last_capability_set(void **state) {
    CRINIT_PARAM_UNUSED(state);

    expect_value(__wrap_prctl, op, PR_CAP_AMBIENT);
    will_return(__wrap_prctl, 0);
    assert_int_equal(crinitCapSetAmbient(1uLL << CAP_LAST_CAP), 0);
}

void test_crinitCapSetAmbient_no_capability_set(void **state) {
    CRINIT_PARAM_UNUSED(state);
    assert_int_equal(crinitCapSetAmbient(0), 0);
}
