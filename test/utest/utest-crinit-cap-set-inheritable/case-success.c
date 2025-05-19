// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCapSetInheritable(), successful execution.
 */

#include "capabilities.h"
#include "common.h"
#include "unit_test.h"
#include "utest-crinit-cap-set-inheritable.h"

void test_crinitCapSetInheritable(void **state) {
    CRINIT_PARAM_UNUSED(state);

    uint64_t capMask = 0x11;
    assert_int_equal(crinitCapSetInheritable(capMask), 0);
}

void test_crinitCapSetInheritable_last_supported(void **state) {
    CRINIT_PARAM_UNUSED(state);

    uint64_t capMask = 1uLL << CAP_LAST_CAP;  // highest possible capability set
    assert_int_equal(crinitCapSetInheritable(capMask), 0);
}
