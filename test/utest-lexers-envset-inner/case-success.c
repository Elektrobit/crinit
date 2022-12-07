/**
 * @file case-success.c
 * @brief Unit test for EBCL_crinitTaskAdd(), successful execution.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "common.h"
#include "lexers.h"
#include "string.h"
#include "unit_test.h"
#include "utest-lexers-envset-inner.h"

#define EBCL_UTEST_DUMMY_ENVVAR_NAME "SOME_VAR"

void EBCL_envVarInnerLexTestSuccess(void **state) {
    EBCL_PARAM_UNUSED(state);

    const char *mbegin = NULL, *mend = NULL;
    const char *cpyStr = "COPYTHIS";
    const char *escSeq = "\\n";
    const char *escSeqHex = "\\x7e";
    const char *var = "${" EBCL_UTEST_DUMMY_ENVVAR_NAME "}";
    const char *end = "";

    const char *s = NULL;

    /* Should consume/match a single character. */
    s = cpyStr;
    assert_int_equal(EBCL_envVarInnerLex(&s, &mbegin, &mend), EBCL_TK_CPY);
    assert_ptr_equal(mbegin, cpyStr);
    assert_ptr_equal(mend, cpyStr + 1);
    assert_ptr_equal(mend, s);

    /* Should match a regular (backslash plus single character) escape sequence. */
    s = escSeq;
    assert_int_equal(EBCL_envVarInnerLex(&s, &mbegin, &mend), EBCL_TK_ESC);
    assert_ptr_equal(mbegin, escSeq);
    assert_ptr_equal(mend, escSeq + strlen(escSeq));
    assert_ptr_equal(mend, s);

    /* Should consume a hexadecimal escape sequence and match its two-digit hexadecimal code. */
    s = escSeqHex;
    assert_int_equal(EBCL_envVarInnerLex(&s, &mbegin, &mend), EBCL_TK_ESCX);
    assert_ptr_equal(mbegin, escSeqHex + 2);
    assert_ptr_equal(mend, escSeqHex + 4);
    assert_ptr_equal(mend, s);

    /* Should consume the whole variable to expand but match only its name. */
    s = var;
    assert_int_equal(EBCL_envVarInnerLex(&s, &mbegin, &mend), EBCL_TK_VAR);
    assert_ptr_equal(mbegin, var + 2);
    assert_ptr_equal(mend, var + 2 + strlen(EBCL_UTEST_DUMMY_ENVVAR_NAME));
    assert_ptr_equal(mend, s - 1);
    assert_true(strlen(mbegin) > strlen(EBCL_UTEST_DUMMY_ENVVAR_NAME));
    assert_memory_equal(mbegin, EBCL_UTEST_DUMMY_ENVVAR_NAME, strlen(EBCL_UTEST_DUMMY_ENVVAR_NAME));

    /* Should match the end-of-string */
    s = end;
    assert_int_equal(EBCL_envVarInnerLex(&s, &mbegin, &mend), EBCL_TK_END);
    assert_ptr_equal(mbegin, end);
    assert_ptr_equal(mend, end + 1);
    assert_ptr_equal(mend, s);
}
