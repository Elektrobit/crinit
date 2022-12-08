/**
 * @file case-null-input.c
 * @brief Unit test for EBCL_envSetParseAndSet() with NULL inputs.
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
#include "utest-envset-parse-and-set.h"

void EBCL_envSetParseAndSetTestNullInput(void **state) {
    EBCL_PARAM_UNUSED(state);

    ebcl_EnvSet_t failureDummy = {NULL, 0, 0};
    ebcl_EnvSet_t successDummy = {NULL, 0, 0};

    const char *envConf = "VAR_NAME \"some val\"";

    assert_int_equal(EBCL_envSetInit(&successDummy, EBCL_ENVSET_INITIAL_SIZE, EBCL_ENVSET_SIZE_INCREMENT), 0);

    assert_int_equal(EBCL_envSetParseAndSet(NULL, NULL), -1);
    assert_int_equal(EBCL_envSetParseAndSet(NULL, envConf), -1);
    assert_int_equal(EBCL_envSetParseAndSet(&successDummy, NULL), -1);
    assert_int_equal(EBCL_envSetParseAndSet(&failureDummy, envConf), -1);

    assert_int_equal(EBCL_envSetDestroy(&successDummy), 0);
}
