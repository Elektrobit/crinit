// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitKernelCmdlineLex(), successful execution.
 */

#include "common.h"
#include "lexers.h"
#include "string.h"
#include "unit_test.h"
#include "utest-lexers-kcmdline.h"

void crinitKernelCmdlineLexTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *keyBegin = NULL, *keyEnd = NULL, *valBegin = NULL, *valEnd = NULL;
    const char *tokOptUq = "crinit.foo=bar other.var=val";
    const char *tokOptDq = "crinit.foo=\"bar baz\" other.var=val";
    const char *tokWspc = "   \t  ";
    const char *tokNothing = "notcrinit.nothing_to=\"see here\"";
    const char *tokEnd = "";

    const char *s = NULL;

    /* Should consume/match a single character. */
    s = tokNothing;
    assert_int_equal(crinitKernelCmdlineLex(&s, &keyBegin, &keyEnd, &valBegin, &valEnd), CRINIT_TK_CPY);
    assert_ptr_equal(keyBegin, tokNothing);
    assert_ptr_equal(keyEnd, tokNothing + 1);
    assert_ptr_equal(keyEnd, s);

    /* Should match and consume whitespace.*/
    s = tokWspc;
    assert_int_equal(crinitKernelCmdlineLex(&s, &keyBegin, &keyEnd, &valBegin, &valEnd), CRINIT_TK_WSPC);
    assert_ptr_equal(keyBegin, tokWspc);
    assert_ptr_equal(keyEnd, tokWspc + strlen(tokWspc));
    assert_ptr_equal(keyEnd, s);

    /* Should consume a whole variable with unquoted content and return matches to name and content.*/
    s = tokOptUq;
    assert_int_equal(crinitKernelCmdlineLex(&s, &keyBegin, &keyEnd, &valBegin, &valEnd), CRINIT_TK_VAR);
    assert_ptr_equal(keyBegin, tokOptUq + strlen("crinit."));
    assert_ptr_equal(keyEnd, tokOptUq + strlen("crinit.foo"));
    assert_ptr_equal(valBegin, tokOptUq + strlen("crinit.foo="));
    assert_ptr_equal(valEnd, tokOptUq + strlen("crinit.foo=bar"));
    assert_ptr_equal(tokOptUq + strlen("crinit.foo=bar"), s);

    /* Should consume a whole variable with unquoted content and return matches to name and content (without the
     * quotes.*/
    s = tokOptDq;
    assert_int_equal(crinitKernelCmdlineLex(&s, &keyBegin, &keyEnd, &valBegin, &valEnd), CRINIT_TK_VAR);
    assert_ptr_equal(keyBegin, tokOptDq + strlen("crinit."));
    assert_ptr_equal(keyEnd, tokOptDq + strlen("crinit.foo"));
    assert_ptr_equal(valBegin, tokOptDq + strlen("crinit.foo=\""));
    assert_ptr_equal(valEnd, tokOptDq + strlen("crinit.foo=\"bar baz"));
    assert_ptr_equal(tokOptDq + strlen("crinit.foo=\"bar baz\""), s);

    /* Should match the end-of-string */
    s = tokEnd;
    assert_int_equal(crinitKernelCmdlineLex(&s, &keyBegin, &keyEnd, &valBegin, &valEnd), CRINIT_TK_END);
    assert_ptr_equal(keyBegin, tokEnd);
    assert_ptr_equal(keyEnd, tokEnd + 1);
    assert_ptr_equal(keyEnd, s);
}
