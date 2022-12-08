/**
 * @file case-success.c
 * @brief Unit test for EBCL_envSetSet(), successful execution.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "common.h"
#include "envset.h"
#include "unit_test.h"
#include "utest-envset-set.h"

void EBCL_envSetSetTestSuccess(void **state) {
    EBCL_PARAM_UNUSED(state);

    ebcl_EnvSet_t e = {NULL, 0, 0};
    assert_int_equal(EBCL_envSetInit(&e, 4, 2), 0);
    assert_int_equal(EBCL_envSetSet(&e, "VAR1", "val1"), 0);
    assert_int_equal(EBCL_envSetSet(&e, "VAR2", "val2"), 0);
    assert_int_equal(EBCL_envSetSet(&e, "VAR3", "val3"), 0);

    // Up to here no growth should be necessary
    assert_string_equal(EBCL_envSetGet(&e, "VAR1"), "val1");
    assert_string_equal(EBCL_envSetGet(&e, "VAR2"), "val2");
    assert_string_equal(EBCL_envSetGet(&e, "VAR3"), "val3");

    // For this the envSet will need to grow once.
    assert_int_equal(EBCL_envSetSet(&e, "VAR4", "val4"), 0);
    assert_int_equal(EBCL_envSetSet(&e, "VAR5", "val5"), 0);

    // Check if everything is as expected afterwards
    assert_string_equal(EBCL_envSetGet(&e, "VAR1"), "val1");
    assert_string_equal(EBCL_envSetGet(&e, "VAR2"), "val2");
    assert_string_equal(EBCL_envSetGet(&e, "VAR3"), "val3");
    assert_string_equal(EBCL_envSetGet(&e, "VAR4"), "val4");
    assert_string_equal(EBCL_envSetGet(&e, "VAR5"), "val5");
    assert_int_equal(e.allocSz, 6);
    assert_int_equal(e.allocInc, 2);

    // For this the envSet will need to grow once more.
    assert_int_equal(EBCL_envSetSet(&e, "VAR6", "val6"), 0);
    assert_int_equal(EBCL_envSetSet(&e, "VAR7", "val7"), 0);

    // Check if everything is as expected afterwards
    assert_string_equal(EBCL_envSetGet(&e, "VAR1"), "val1");
    assert_string_equal(EBCL_envSetGet(&e, "VAR2"), "val2");
    assert_string_equal(EBCL_envSetGet(&e, "VAR3"), "val3");
    assert_string_equal(EBCL_envSetGet(&e, "VAR4"), "val4");
    assert_string_equal(EBCL_envSetGet(&e, "VAR5"), "val5");
    assert_string_equal(EBCL_envSetGet(&e, "VAR6"), "val6");
    assert_string_equal(EBCL_envSetGet(&e, "VAR7"), "val7");
    assert_int_equal(e.allocSz, 8);
    assert_int_equal(e.allocInc, 2);

    assert_int_equal(EBCL_envSetDestroy(&e), 0);
}
