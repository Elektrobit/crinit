// SPDX-License-Identifier: MIT
/**
 * @file utest-file-basename-macro.h
 * @brief Header declaring the regression test for the __FILE_BASENAME__ macro.
 */
#ifndef __UTEST_FILE_BASENAME_MACRO_H__
#define __UTEST_FILE_BASENAME_MACRO_H__

/**
 * Regression test for __FILE_BASENAME__ macro.
 *
 * Checks that the macro is present and only shows the file's basename and not its whole path.
 */
void crinitFileBasenameMacroRegressionTest(void **state);

#endif /* __UTEST_FILE_BASENAME_MACRO__ */
