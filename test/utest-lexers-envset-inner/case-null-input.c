/**
 * @file case-null-input.c
 * @brief Unit test for crinitEnvVarInnerLex() with >=1 NULL inputs.
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

void crinitEnvVarInnerLexTestNullInput(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *s = "Dummy string.", *mbegin = NULL, *mend = NULL;

    /* All possible combinations of having at least one NULL parameter input */
    assert_int_equal(crinitEnvVarInnerLex(NULL, NULL, NULL), CRINIT_TK_ERR);
    assert_int_equal(crinitEnvVarInnerLex(NULL, NULL, &mend), CRINIT_TK_ERR);
    assert_int_equal(crinitEnvVarInnerLex(NULL, &mbegin, NULL), CRINIT_TK_ERR);
    assert_int_equal(crinitEnvVarInnerLex(NULL, &mbegin, &mend), CRINIT_TK_ERR);
    assert_int_equal(crinitEnvVarInnerLex(&s, NULL, NULL), CRINIT_TK_ERR);
    assert_int_equal(crinitEnvVarInnerLex(&s, NULL, &mend), CRINIT_TK_ERR);
    assert_int_equal(crinitEnvVarInnerLex(&s, &mbegin, NULL), CRINIT_TK_ERR);
}
