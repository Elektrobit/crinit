// SPDX-License-Identifier: MIT
/**
 * @file utest-lexers-kcmdline.h
 * @brief Header declaring the unit tests for crinitKernelCmdlineLex().
 */
#ifndef __UTEST_LEXERS_KCMDLINE_H__
#define __UTEST_LEXERS_KCMDLINE_H__

/**
 * Tests successful lex-ing of valid inputs.
 *
 * All possible return values of crinitKernelCmdlineLex() save for CRINIT_TK_ERR are triggered using accordingly built
 * input strings.
 */
void crinitKernelCmdlineLexTestSuccess(void **state);
/**
 * Tests detection of NULL pointer inputs.
 *
 * crinitKernelCmdlineLex() shall fail if any pointer input parameter is NULL and/or the string in *s is NULL.
 */
void crinitKernelCmdlineLexTestNullInput(void **state);

#endif /* __UTEST_LEXERS_KCMDLINE_H__ */
