/**
 * @file case-null-input.c
 * @brief Unit test for crinitEnvVarOuterLex() with >=1 NULL inputs.
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

void crinitEnvVarOuterLexTestNullInput(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *s = "Dummy string.", *mbegin = NULL, *mend = NULL;

    /* All possible combinations of having at least one NULL parameter input */
    assert_int_equal(crinitEnvVarOuterLex(NULL, NULL, NULL), CRINIT_TK_ERR);
    assert_int_equal(crinitEnvVarOuterLex(NULL, NULL, &mend), CRINIT_TK_ERR);
    assert_int_equal(crinitEnvVarOuterLex(NULL, &mbegin, NULL), CRINIT_TK_ERR);
    assert_int_equal(crinitEnvVarOuterLex(NULL, &mbegin, &mend), CRINIT_TK_ERR);
    assert_int_equal(crinitEnvVarOuterLex(&s, NULL, NULL), CRINIT_TK_ERR);
    assert_int_equal(crinitEnvVarOuterLex(&s, NULL, &mend), CRINIT_TK_ERR);
    assert_int_equal(crinitEnvVarOuterLex(&s, &mbegin, NULL), CRINIT_TK_ERR);
}
