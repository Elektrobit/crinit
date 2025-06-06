// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCapConvertToBitmask(), successful execution.
 */

#include "capabilities.h"
#include "common.h"
#include "unit_test.h"
#include "utest-crinit-cap-convert-to-bitmask.h"

void test_crinitCapConvertToBitmask(void **state) {
    CRINIT_PARAM_UNUSED(state);
    uint64_t bitmask;

    const char *caps = "CAP_KILL";
    assert_int_equal(crinitCapConvertToBitmask(&bitmask, caps), 0);
    assert_int_equal(bitmask, 0x20);

    caps = "CAP_KILL CAP_CHOWN";
    assert_int_equal(crinitCapConvertToBitmask(&bitmask, caps), 0);
    assert_int_equal(bitmask, 0x21);

    caps = "";
    assert_int_equal(crinitCapConvertToBitmask(&bitmask, caps), 0);
}
