/**
 * @file case-null-input.c
 * @brief Unit test for EBCL_envVarInnerLex() with >=1 NULL inputs.
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
#include "utest-lexers-envset-inner.h"

void EBCL_envVarInnerLexTestNullInput(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *s = "Dummy string.", *mbegin = NULL, *mend = NULL;

    /* All possible combinations of having at least one NULL parameter input */
    assert_int_equal(EBCL_envVarInnerLex(NULL, NULL, NULL), EBCL_TK_ERR);
    assert_int_equal(EBCL_envVarInnerLex(NULL, NULL, &mend), EBCL_TK_ERR);
    assert_int_equal(EBCL_envVarInnerLex(NULL, &mbegin, NULL), EBCL_TK_ERR);
    assert_int_equal(EBCL_envVarInnerLex(NULL, &mbegin, &mend), EBCL_TK_ERR);
    assert_int_equal(EBCL_envVarInnerLex(&s, NULL, NULL), EBCL_TK_ERR);
    assert_int_equal(EBCL_envVarInnerLex(&s, NULL, &mend), EBCL_TK_ERR);
    assert_int_equal(EBCL_envVarInnerLex(&s, &mbegin, NULL), EBCL_TK_ERR);
}
