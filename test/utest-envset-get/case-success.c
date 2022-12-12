/**
 * @file case-success.c
 * @brief Unit test for EBCL_envSetGet(), successful execution.
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
#include "utest-envset-get.h"

void EBCL_envSetGetTestSuccess(void **state) {
    EBCL_PARAM_UNUSED(state);

    ebcl_EnvSet_t e = {NULL, 0, 0};
    assert_int_equal(EBCL_envSetInit(&e, EBCL_ENVSET_INITIAL_SIZE, EBCL_ENVSET_SIZE_INCREMENT), 0);
    assert_int_equal(EBCL_envSetSet(&e, "VAR1", "val1"), 0);
    assert_int_equal(EBCL_envSetSet(&e, "VAR2", "val2"), 0);
    assert_int_equal(EBCL_envSetSet(&e, "VAR3", "val3"), 0);

    assert_string_equal(EBCL_envSetGet(&e, "VAR1"), "val1");
    assert_string_equal(EBCL_envSetGet(&e, "VAR2"), "val2");
    assert_string_equal(EBCL_envSetGet(&e, "VAR3"), "val3");

    assert_int_equal(EBCL_envSetDestroy(&e), 0);
}
