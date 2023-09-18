// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitEnvVarInnerLex(), successful execution.
 */

#include "common.h"
#include "lexers.h"
#include "string.h"
#include "unit_test.h"
#include "utest-lexers-envset-inner.h"

#define CRINIT_UTEST_DUMMY_ENVVAR_NAME "SOME_VAR"

void crinitEnvVarInnerLexTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *mbegin = NULL, *mend = NULL;
    const char *cpyStr = "COPYTHIS";
    const char *escSeq = "\\n";
    const char *escSeqHex = "\\x7e";
    const char *var = "${" CRINIT_UTEST_DUMMY_ENVVAR_NAME "}";
    const char *end = "";

    const char *s = NULL;

    /* Should consume/match a single character. */
    s = cpyStr;
    assert_int_equal(crinitEnvVarInnerLex(&s, &mbegin, &mend), CRINIT_TK_CPY);
    assert_ptr_equal(mbegin, cpyStr);
    assert_ptr_equal(mend, cpyStr + 1);
    assert_ptr_equal(mend, s);

    /* Should match a regular (backslash plus single character) escape sequence. */
    s = escSeq;
    assert_int_equal(crinitEnvVarInnerLex(&s, &mbegin, &mend), CRINIT_TK_ESC);
    assert_ptr_equal(mbegin, escSeq);
    assert_ptr_equal(mend, escSeq + strlen(escSeq));
    assert_ptr_equal(mend, s);

    /* Should consume a hexadecimal escape sequence and match its two-digit hexadecimal code. */
    s = escSeqHex;
    assert_int_equal(crinitEnvVarInnerLex(&s, &mbegin, &mend), CRINIT_TK_ESCX);
    assert_ptr_equal(mbegin, escSeqHex + 2);
    assert_ptr_equal(mend, escSeqHex + 4);
    assert_ptr_equal(mend, s);

    /* Should consume the whole variable to expand but match only its name. */
    s = var;
    assert_int_equal(crinitEnvVarInnerLex(&s, &mbegin, &mend), CRINIT_TK_VAR);
    assert_ptr_equal(mbegin, var + 2);
    assert_ptr_equal(mend, var + 2 + strlen(CRINIT_UTEST_DUMMY_ENVVAR_NAME));
    assert_ptr_equal(mend, s - 1);
    assert_true(strlen(mbegin) > strlen(CRINIT_UTEST_DUMMY_ENVVAR_NAME));
    assert_memory_equal(mbegin, CRINIT_UTEST_DUMMY_ENVVAR_NAME, strlen(CRINIT_UTEST_DUMMY_ENVVAR_NAME));

    /* Should match the end-of-string */
    s = end;
    assert_int_equal(crinitEnvVarInnerLex(&s, &mbegin, &mend), CRINIT_TK_END);
    assert_ptr_equal(mbegin, end);
    assert_ptr_equal(mend, end + 1);
    assert_ptr_equal(mend, s);
}
