/**
 * @file case-lexer-error.c
 * @brief Unit test for crinitEnvVarInnerLex() testing error handling of the lexer.
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

void crinitEnvVarInnerLexTestLexerError(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *mbegin = NULL, *mend = NULL;
    const char *errorSingleSlashAtEnd = "\\";

    assert_int_equal(crinitEnvVarInnerLex(&errorSingleSlashAtEnd, &mbegin, &mend), CRINIT_TK_ERR);
}
