/**
 * @file case-confpath-null.c
 * @brief Implementation of a unit test for EBCL_crinitTaskAdd() with an invalid config file path parameter.
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

#define TEST_FORCE_DEPS "@foobar"

void EBCL_crinitTaskAddTestConfPathNull(void **state) {
    EBCL_PARAM_UNUSED(state);

    assert_int_equal(EBCL_crinitTaskAdd(NULL, false, TEST_FORCE_DEPS), -1);
}
