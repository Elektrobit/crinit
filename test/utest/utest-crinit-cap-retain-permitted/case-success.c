// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCapRetainPermitted(), successful execution.
 */

#include <sys/prctl.h>

#include "capabilities.h"
#include "common.h"
#include "unit_test.h"
#include "utest-crinit-cap-retain-permitted.h"

void test_crinitCapRetainPermitted(void **state) {
    CRINIT_PARAM_UNUSED(state);

    expect_value(__wrap_prctl, op, PR_GET_SECUREBITS);
    will_return(__wrap_prctl, 0);
    expect_value(__wrap_prctl, op, PR_SET_SECUREBITS);
    will_return(__wrap_prctl, 0);
    assert_int_equal(crinitCapRetainPermitted(), 0);
}
