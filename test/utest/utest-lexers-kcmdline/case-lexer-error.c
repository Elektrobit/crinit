// SPDX-License-Identifier: MIT
/**
 * @file case-lexer-error.c
 * @brief Unit test for crinitKernelCmdlineLex() testing error handling of the lexer.
 */

#include "common.h"
#include "lexers.h"
#include "unit_test.h"
#include "utest-lexers-kcmdline.h"

void crinitKernelCmdlineLexTestLexerError(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *keyBegin = NULL, *keyEnd = NULL, *valBegin = NULL, *valEnd = NULL;
    const char *errorSingleSlashAtEnd = "\\";

    assert_int_equal(crinitKernelCmdlineLex(&errorSingleSlashAtEnd, &keyBegin, &keyEnd, &valBegin, &valEnd),
                     CRINIT_TK_ERR);
}
