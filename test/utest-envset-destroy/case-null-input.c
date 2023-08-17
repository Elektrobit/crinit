// SPDX-License-Identifier: MIT
/**
 * @file case-null-input.c
 * @brief Unit test for crinitEnvSetDestroy() with a NULL input.
 */

#include "common.h"
#include "envset.h"
#include "unit_test.h"
#include "utest-envset-destroy.h"

void crinitEnvSetDestroyTestNullInput(void **state) {
    CRINIT_PARAM_UNUSED(state);

    expect_any(__wrap_crinitErrPrintFFL, format);
    assert_int_equal(crinitEnvSetDestroy(NULL), -1);
}
