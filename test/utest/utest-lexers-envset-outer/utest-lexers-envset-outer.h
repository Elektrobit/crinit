// SPDX-License-Identifier: MIT
/**
 * @file utest-lexers-envset-outer.h
 * @brief Header declaring the unit tests for crinitEnvVarOuterLex().
 */
#ifndef __UTEST_LEXERS_ENVSET_OUTER_H__
#define __UTEST_LEXERS_ENVSET_OUTER_H__

/**
 * Tests successful lex-ing of valid inputs.
 *
 * All possible return values of crinitEnvVarOuterLex() save for CRINIT_TK_ERR are triggered using accordingly built
 * input strings.
 */
void crinitEnvVarOuterLexTestSuccess(void **state);
/**
 * Tests detection of NULL pointer inputs.
 *
 * crinitEnvVarOuterLex() shall fail if any pointer input parameter is NULL and/or the string in *s is NULL.
 */
void crinitEnvVarOuterLexTestNullInput(void **state);
/**
 * Tests cases leading to a lexer error (CRINIT_TK_ERR).
 *
 * For crinitEnvVarOuterLex() that can only happen if the input string starts with a character not allowed in an
 * environment key _AND_ is not an opening double quote.
 */
void crinitEnvVarOuterLexTestLexerError(void **state);

#endif /* __UTEST_LEXERS_ENVSET_OUTER_H__ */
