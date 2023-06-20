/**
 * @file case-wrong-input.c
 * @brief Unit test for crinitConfConvToEnvSetMember(), handling of invalid string input.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "common.h"
#include "confconv.h"
#include "envset.h"
#include "unit_test.h"
#include "utest-confconv-envset.h"

void crinitConfConvToEnvSetMemberTestWrongInput(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *unquoted = "VANILLA_VAR That is tasty.", *wronglyQuoted = "\"VANILLA_VAR\" That is tasty.",
               *noKey = "\"That is tasty.\"", *noVal = "VANILLA_VAR";

    crinitEnvSet_t e = {NULL, 0, 0};
    assert_int_equal(crinitEnvSetInit(&e, CRINIT_ENVSET_INITIAL_SIZE, CRINIT_ENVSET_SIZE_INCREMENT), 0);

    assert_int_equal(crinitConfConvToEnvSetMember(&e, unquoted), -1);
    assert_int_equal(crinitConfConvToEnvSetMember(&e, wronglyQuoted), -1);
    assert_int_equal(crinitConfConvToEnvSetMember(&e, noKey), -1);
    assert_int_equal(crinitConfConvToEnvSetMember(&e, noVal), -1);

    assert_int_equal(crinitEnvSetDestroy(&e), 0);
}
