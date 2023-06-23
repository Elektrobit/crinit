/**
 * @file utest-file-basename-macro.h
 * @brief Header declaring the regression test for the __FILE_BASENAME__ macro.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
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
