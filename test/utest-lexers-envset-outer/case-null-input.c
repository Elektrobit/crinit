/**
 * @file case-null-input.c
 * @brief Unit test for EBCL_envVarOuterLex() with >=1 NULL inputs.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "common.h"
#include "lexers.h"
#include "unit_test.h"
#include "utest-lexers-envset-outer.h"

void EBCL_envVarOuterLexTestNullInput(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *s = "Dummy string.", *mbegin = NULL, *mend = NULL;

    /* All possible combinations of having at least one NULL parameter input */
    assert_int_equal(EBCL_envVarOuterLex(NULL, NULL, NULL), EBCL_TK_ERR);
    assert_int_equal(EBCL_envVarOuterLex(NULL, NULL, &mend), EBCL_TK_ERR);
    assert_int_equal(EBCL_envVarOuterLex(NULL, &mbegin, NULL), EBCL_TK_ERR);
    assert_int_equal(EBCL_envVarOuterLex(NULL, &mbegin, &mend), EBCL_TK_ERR);
    assert_int_equal(EBCL_envVarOuterLex(&s, NULL, NULL), EBCL_TK_ERR);
    assert_int_equal(EBCL_envVarOuterLex(&s, NULL, &mend), EBCL_TK_ERR);
    assert_int_equal(EBCL_envVarOuterLex(&s, &mbegin, NULL), EBCL_TK_ERR);
}
