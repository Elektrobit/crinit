/**
 * @file case-confpath-null.c
 * @brief Unit test for crinitClientTaskAdd() with confFilePath as NULL.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
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
