// SPDX-License-Identifier: MIT
/**
 * @file case-null-input.c
 * @brief Unit test for crinitEnvSetGet() with NULL inputs.
 */

#include "common.h"
#include "envset.h"
#include "unit_test.h"
#include "utest-envset-get.h"

void crinitEnvSetGetTestNullInput(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitEnvSet_t failureDummy = {NULL, 0, 0};
    crinitEnvSet_t successDummy = {NULL, 0, 0};

    assert_int_equal(crinitEnvSetInit(&successDummy, CRINIT_ENVSET_INITIAL_SIZE, CRINIT_ENVSET_SIZE_INCREMENT), 0);
    assert_ptr_equal(crinitEnvSetGet(&successDummy, NULL), NULL);
    assert_ptr_equal(crinitEnvSetGet(NULL, "VARNAME"), NULL);
    assert_ptr_equal(crinitEnvSetGet(NULL, NULL), NULL);
    assert_ptr_equal(crinitEnvSetGet(&failureDummy, "VARNAME"), NULL);
    assert_int_equal(crinitEnvSetDestroy(&successDummy), 0);
}
