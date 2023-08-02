// SPDX-License-Identifier: MIT
/**
 * @file case-lexer-error.c
 * @brief Unit test for crinitEnvVarOuterLex() testing error handling of the lexer.
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
