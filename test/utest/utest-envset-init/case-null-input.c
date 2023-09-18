// SPDX-License-Identifier: MIT
/**
 * @file case-null-input.c
 * @brief Unit test for crinitEnvSetInit() with a NULL input.
 */

#include "common.h"
#include "envset.h"
#include "unit_test.h"
#include "utest-envset-init.h"

void crinitEnvSetInitTestNullInput(void **state) {
    CRINIT_PARAM_UNUSED(state);

    expect_any(__wrap_crinitErrPrintFFL, format);
    assert_int_equal(crinitEnvSetInit(NULL, CRINIT_ENVSET_INITIAL_SIZE, CRINIT_ENVSET_SIZE_INCREMENT), -1);
}
