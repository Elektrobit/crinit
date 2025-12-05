// SPDX-License-Identifier: MIT
/**
 * @file utest-lexers-envset-outer.h
 * @brief Header declaring the unit tests for crinitEnvVarOuterLex().
 */
#ifndef __UTEST_TIMER_PARSER_H__
#define __UTEST_TIMER_PARSER_H__

/**
 * Tests successful parsing of valid inputs.
 */
void crinitTimerParserTestSuccess(void **state);
/**
 * Tests fail parsing of invalid inputs.
 */
void crinitTimerParserTestError(void **state);
/**
 * Test fail parsing with NULL parameters
 */
void crinitTimerParserTestNullParam(void **state);

#endif /* __UTEST_TIMER_PARSER_H__ */
