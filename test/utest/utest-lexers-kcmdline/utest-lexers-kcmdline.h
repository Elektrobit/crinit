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
/**
 * Tests cases leading to a lexer error (CRINIT_TK_ERR).
 *
 * For crinitKernelCmdlineLex() that can only happen if the input string starts with a character not allowed in an
 * variable name and is not an opening double quote.
 */
void crinitKernelCmdlineLexTestLexerError(void **state);

#endif /* __UTEST_LEXERS_KCMDLINE_H__ */
