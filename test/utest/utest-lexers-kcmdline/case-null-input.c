// SPDX-License-Identifier: MIT
/**
 * @file case-null-input.c
 * @brief Unit test for crinitKernelCmdlineLex() with NULL inputs.
 */

#include "common.h"
#include "lexers.h"
#include "unit_test.h"
#include "utest-lexers-kcmdline.h"

void crinitKernelCmdlineLexTestNullInput(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *s = "Dummy string.", *keyBegin = NULL, *keyEnd = NULL, *valBegin = NULL, *valEnd = NULL;

    /* As we have 5 different variables, we're not testing every possible combination but just each singular occurence
     * of NULL */
    assert_int_equal(crinitKernelCmdlineLex(NULL, &keyBegin, &keyEnd, &valBegin, &valEnd), CRINIT_TK_ERR);
    assert_int_equal(crinitKernelCmdlineLex(&s, NULL, &keyEnd, &valBegin, &valEnd), CRINIT_TK_ERR);
    assert_int_equal(crinitKernelCmdlineLex(&s, &keyBegin, NULL, &valBegin, &valEnd), CRINIT_TK_ERR);
    assert_int_equal(crinitKernelCmdlineLex(&s, &keyBegin, &keyEnd, NULL, &valEnd), CRINIT_TK_ERR);
    assert_int_equal(crinitKernelCmdlineLex(&s, &keyBegin, &keyEnd, &valBegin, NULL), CRINIT_TK_ERR);
    /* Also check case where s would point to a NULL pointer */
    assert_int_equal(crinitKernelCmdlineLex(&keyBegin, &keyBegin, &keyEnd, &valBegin, &valEnd), CRINIT_TK_ERR);
}
