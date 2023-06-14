/**
 * @file case-lexer-error.c
 * @brief Unit test for crinitEnvVarOuterLex() testing error handling of the lexer.
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

void crinitEnvVarOuterLexTestLexerError(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *mbegin = NULL, *mend = NULL;
    const char *errorInvalidCharStart = "$key";
    const char *errorKeyStartWithNumber = "0key";

    assert_int_equal(crinitEnvVarOuterLex(&errorInvalidCharStart, &mbegin, &mend), CRINIT_TK_ERR);
    assert_int_equal(crinitEnvVarOuterLex(&errorKeyStartWithNumber, &mbegin, &mend), CRINIT_TK_ERR);
}
