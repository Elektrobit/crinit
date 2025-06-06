// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCapGetInheritable(), successful execution.
 */

#include "capabilities.h"
#include "common.h"
#include "unit_test.h"
#include "utest-crinit-cap-get-inheritable.h"

void test_crinitCapGetInheritable_invalid_capability_pointer(void **state) {
    CRINIT_PARAM_UNUSED(state);
    pid_t pid = 0;

    assert_int_equal(crinitCapGetInheritable(pid, NULL), -1);
}
