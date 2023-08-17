// SPDX-License-Identifier: MIT
/**
 * @file case-null-input.c
 * @brief Unit test for crinitEnvSetDup() with a NULL input.
 */

#include "common.h"
#include "envset.h"
#include "unit_test.h"
#include "utest-envset-dup.h"

void crinitEnvSetDupTestNullInput(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitEnvSet_t successDummy = {(char **)0xdeadc0de, 0, 0}, failureDummy = {NULL, 0, 0};
    assert_int_equal(crinitEnvSetDup(&successDummy, NULL), -1);
    assert_int_equal(crinitEnvSetDup(&successDummy, &failureDummy), -1);
    assert_int_equal(crinitEnvSetDup(NULL, &successDummy), -1);
    assert_int_equal(crinitEnvSetDup(NULL, NULL), -1);
}
