/**
 * @file utest-lexers-envset-inner.h
 * @brief Header declaring the unit tests for crinitEnvVarInnerLex().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __UTEST_LEXERS_ENVSET_INNER_H__
#define __UTEST_LEXERS_ENVSET_INNER_H__

/**
 * Tests successful lex-ing of valid inputs.
 *
 * All possible return values of crinitEnvVarInnerLex() save for CRINIT_TK_ERR are triggered using accordingly built input
 * strings.
 */
void crinitEnvVarInnerLexTestSuccess(void **state);
/**
 * Tests detection of NULL pointer inputs.
 *
 * crinitEnvVarInnerLex() shall fail if any pointer input parameter is NULL and/or the string in *s is NULL.
 */
void crinitEnvVarInnerLexTestNullInput(void **state);
/**
 * Tests cases leading to a lexer error (CRINIT_TK_ERR).
 *
 * For crinitEnvVarInnerLex() that can only happen if the input string starts with a character not allowed in an
 * environment key _AND_ is not an opening double quote.
 */
void crinitEnvVarInnerLexTestLexerError(void **state);

#endif /* __UTEST_LEXERS_ENVSET_INNER_H__ */
