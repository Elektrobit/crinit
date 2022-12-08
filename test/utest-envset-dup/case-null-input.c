/**
 * @file case-null-input.c
 * @brief Unit test for EBCL_envSetDup() with a NULL input.
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
#include "utest-envset-dup.h"

void EBCL_envSetDupTestNullInput(void **state) {
    EBCL_PARAM_UNUSED(state);

    ebcl_EnvSet_t successDummy = {(char **)0xdeadc0de, 0, 0}, failureDummy = {NULL, 0, 0};
    assert_int_equal(EBCL_envSetDup(&successDummy, NULL), -1);
    assert_int_equal(EBCL_envSetDup(&successDummy, &failureDummy), -1);
    assert_int_equal(EBCL_envSetDup(NULL, &successDummy), -1);
    assert_int_equal(EBCL_envSetDup(NULL, NULL), -1);
}
