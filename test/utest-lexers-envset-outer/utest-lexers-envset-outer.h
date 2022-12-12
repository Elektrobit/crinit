/**
 * @file utest-lexers-envset-outer.h
 * @brief Header declaring the unit tests for EBCL_envVarOuterLex().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __UTEST_LEXERS_ENVSET_OUTER_H__
#define __UTEST_LEXERS_ENVSET_OUTER_H__

/**
 * Tests successful lex-ing of valid inputs.
 *
 * All possible return values of EBCL_envVarOuterLex() save for EBCL_TK_ERR are triggered using accordingly built input
 * strings.
 */
void EBCL_envVarOuterLexTestSuccess(void **state);
/**
 * Tests detection of NULL pointer inputs.
 *
 * EBCL_envVarOuterLex() shall fail if any pointer input parameter is NULL and/or the string in *s is NULL.
 */
void EBCL_envVarOuterLexTestNullInput(void **state);
/**
 * Tests cases leading to a lexer error (EBCL_TK_ERR).
 *
 * For EBCL_envVarOuterLex() that can only happen if the input string starts with a character not allowed in an
 * environment key _AND_ is not an opening double quote.
 */
void EBCL_envVarOuterLexTestLexerError(void **state);

#endif /* __UTEST_LEXERS_ENVSET_OUTER_H__ */
