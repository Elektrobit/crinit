// SPDX-License-Identifier: MIT
/**
 * @file case-null-input.c
 * @brief Unit test for crinitEnvVarOuterLex() with >=1 NULL inputs.
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
    /* Also check case where s would point to a NULL pointer */
    assert_int_equal(crinitEnvVarOuterLex(&mbegin, &mbegin, &mend), CRINIT_TK_ERR);
}
