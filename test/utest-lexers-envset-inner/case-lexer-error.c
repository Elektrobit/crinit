/**
 * @file case-lexer-error.c
 * @brief Unit test for EBCL_envVarInnerLex() testing error handling of the lexer.
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

void EBCL_envVarInnerLexTestLexerError(void **state) {
    EBCL_PARAM_UNUSED(state);

    const char *mbegin = NULL, *mend = NULL;
    const char *errorSingleSlashAtEnd = "\\";

    assert_int_equal(EBCL_envVarInnerLex(&errorSingleSlashAtEnd, &mbegin, &mend), EBCL_TK_ERR);
}
