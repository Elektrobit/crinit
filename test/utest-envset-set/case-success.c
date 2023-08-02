// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitEnvSetSet(), successful execution.
 */

#include "common.h"
#include "envset.h"
#include "unit_test.h"
#include "utest-envset-set.h"

void crinitEnvSetSetTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitEnvSet_t e = {NULL, 0, 0};
    assert_int_equal(crinitEnvSetInit(&e, 4, 2), 0);
    assert_int_equal(crinitEnvSetSet(&e, "VAR1", "val1"), 0);
    assert_int_equal(crinitEnvSetSet(&e, "VAR2", "val2"), 0);
    assert_int_equal(crinitEnvSetSet(&e, "VAR3", "val3"), 0);

    // Up to here no growth should be necessary
    assert_string_equal(crinitEnvSetGet(&e, "VAR1"), "val1");
    assert_string_equal(crinitEnvSetGet(&e, "VAR2"), "val2");
    assert_string_equal(crinitEnvSetGet(&e, "VAR3"), "val3");

    // For this the envSet will need to grow once.
    assert_int_equal(crinitEnvSetSet(&e, "VAR4", "val4"), 0);
    assert_int_equal(crinitEnvSetSet(&e, "VAR5", "val5"), 0);

    // Check if everything is as expected afterwards
    assert_string_equal(crinitEnvSetGet(&e, "VAR1"), "val1");
    assert_string_equal(crinitEnvSetGet(&e, "VAR2"), "val2");
    assert_string_equal(crinitEnvSetGet(&e, "VAR3"), "val3");
    assert_string_equal(crinitEnvSetGet(&e, "VAR4"), "val4");
    assert_string_equal(crinitEnvSetGet(&e, "VAR5"), "val5");
    assert_int_equal(e.allocSz, 6);
    assert_int_equal(e.allocInc, 2);

    // For this the envSet will need to grow once more.
    assert_int_equal(crinitEnvSetSet(&e, "VAR6", "val6"), 0);
    assert_int_equal(crinitEnvSetSet(&e, "VAR7", "val7"), 0);

    // Check if everything is as expected afterwards
    assert_string_equal(crinitEnvSetGet(&e, "VAR1"), "val1");
    assert_string_equal(crinitEnvSetGet(&e, "VAR2"), "val2");
    assert_string_equal(crinitEnvSetGet(&e, "VAR3"), "val3");
    assert_string_equal(crinitEnvSetGet(&e, "VAR4"), "val4");
    assert_string_equal(crinitEnvSetGet(&e, "VAR5"), "val5");
    assert_string_equal(crinitEnvSetGet(&e, "VAR6"), "val6");
    assert_string_equal(crinitEnvSetGet(&e, "VAR7"), "val7");
    assert_int_equal(e.allocSz, 8);
    assert_int_equal(e.allocInc, 2);

    assert_int_equal(crinitEnvSetDestroy(&e), 0);
}
