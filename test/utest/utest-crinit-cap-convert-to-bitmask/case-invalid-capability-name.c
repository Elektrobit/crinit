// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCapConvertToBitmask(), handling of invalid capability names.
 */

#include "capabilities.h"
#include "common.h"
#include "unit_test.h"
#include "utest-crinit-cap-convert-to-bitmask.h"

void test_crinitCapConvertToBitmask_invalid_capability_name(void **state) {
    CRINIT_PARAM_UNUSED(state);
    uint64_t bitmask;

    const char *caps = "CAP_KIL";
    assert_int_equal(crinitCapConvertToBitmask(&bitmask, caps), -1);

    caps = "CAP_KILL CAP_CHOWN CAP_FSETI";
    assert_int_equal(crinitCapConvertToBitmask(&bitmask, caps), -1);
}
