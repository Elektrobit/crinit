/**
 * @file case-wrong-input.c
 * @brief Unit test for EBCL_envSetParseAndSet(), handling of invalid string input.
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

void EBCL_envSetParseAndSetTestWrongInput(void **state) {
    EBCL_PARAM_UNUSED(state);

    const char *unquoted = "VANILLA_VAR That is tasty.", *wronglyQuoted = "\"VANILLA_VAR\" That is tasty.",
               *noKey = "\"That is tasty.\"", *noVal = "VANILLA_VAR";

    ebcl_EnvSet_t e = {NULL, 0, 0};
    assert_int_equal(EBCL_envSetInit(&e, EBCL_ENVSET_INITIAL_SIZE, EBCL_ENVSET_SIZE_INCREMENT), 0);

    assert_int_equal(EBCL_envSetParseAndSet(&e, unquoted), -1);
    assert_int_equal(EBCL_envSetParseAndSet(&e, wronglyQuoted), -1);
    assert_int_equal(EBCL_envSetParseAndSet(&e, noKey), -1);
    assert_int_equal(EBCL_envSetParseAndSet(&e, noVal), -1);
 
    assert_int_equal(EBCL_envSetDestroy(&e), 0);
}
