// SPDX-License-Identifier: MIT
/**
 * @file case-invalid-capability.c
 * @brief Unit test for crinitCapSetInheritable(), failing execution, invalid capabilities.
 */

#include "capabilities.h"
#include "common.h"
#include "unit_test.h"
#include "utest-crinit-cap-set-inheritable.h"

void test_crinitCapSetInheritable_invalid_capability(void **state) {
    CRINIT_PARAM_UNUSED(state);

    uint64_t capMask = 0x23;
    assert_int_equal(crinitCapSetInheritable(capMask), -1);
}

void test_crinitCapSetInheritable_invalid_capability_range(void **state) {
    CRINIT_PARAM_UNUSED(state);

    uint64_t capMask = 1uLL << (CAP_LAST_CAP + 1);  // first invalid capability set
    assert_int_equal(crinitCapSetInheritable(capMask), -1);
}
