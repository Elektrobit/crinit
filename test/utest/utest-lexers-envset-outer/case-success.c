// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitClientTaskAdd(), successful execution.
 */

#include "common.h"
#include "lexers.h"
#include "string.h"
#include "unit_test.h"
#include "utest-lexers-envset-outer.h"

#define CRINIT_DUMMY_ENVIRONMENT_VALUE "Some quoted value with $VAR\\x2e"

void crinitEnvVarOuterLexTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *mbegin = NULL, *mend = NULL;
    const char *validEnvKey = "SUPER_key9000";
    const char *validEnvVal = "\"" CRINIT_DUMMY_ENVIRONMENT_VALUE "\"";
    const char *wSpc = "    ";
    const char *end = "";

    const char *s = NULL;

    /* Should be matched as an environment key */
    s = validEnvKey;
    assert_int_equal(crinitEnvVarOuterLex(&s, &mbegin, &mend), CRINIT_TK_ENVKEY);
    assert_ptr_equal(mbegin, validEnvKey);
    assert_ptr_equal(mend, validEnvKey + strlen(validEnvKey));
    assert_ptr_equal(mend, s);

    /* Should be matched as an environment value. The quotes should be consumed but not be contained in the match
     * (between mbegin and mend) */
    s = validEnvVal;
    assert_int_equal(crinitEnvVarOuterLex(&s, &mbegin, &mend), CRINIT_TK_ENVVAL);
    assert_ptr_equal(mbegin, validEnvVal + 1);
    assert_ptr_equal(mend, validEnvVal + strlen(validEnvVal) - 1);
    assert_ptr_equal(mend, s - 1);
    assert_true(strlen(mbegin) > strlen(CRINIT_DUMMY_ENVIRONMENT_VALUE));
    assert_memory_equal(mbegin, CRINIT_DUMMY_ENVIRONMENT_VALUE, strlen(CRINIT_DUMMY_ENVIRONMENT_VALUE));

    /* Should consume/match all whitespace */
    s = wSpc;
    assert_int_equal(crinitEnvVarOuterLex(&s, &mbegin, &mend), CRINIT_TK_WSPC);
    assert_ptr_equal(mbegin, wSpc);
    assert_ptr_equal(mend, wSpc + strlen(wSpc));
    assert_ptr_equal(mend, s);

    /* Should match the end-of-string */
    s = end;
    assert_int_equal(crinitEnvVarOuterLex(&s, &mbegin, &mend), CRINIT_TK_END);
    assert_ptr_equal(mbegin, end);
    assert_ptr_equal(mend, end + 1);
    assert_ptr_equal(mend, s);
}
