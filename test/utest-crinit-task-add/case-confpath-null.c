// SPDX-License-Identifier: MIT
/**
 * @file case-confpath-null.c
 * @brief Unit test for crinitClientTaskAdd() with confFilePath as NULL.
 */

#include "common.h"
#include "crinit-client.h"
#include "unit_test.h"
#include "utest-crinit-task-add.h"

#define TEST_FORCE_DEPS "foo:wait"

void crinitClientTaskAddTestConfPathNull(void **state) {
    CRINIT_PARAM_UNUSED(state);

    assert_int_equal(crinitClientTaskAdd(NULL, false, TEST_FORCE_DEPS), -1);
}
