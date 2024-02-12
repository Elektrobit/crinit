// SPDX-License-Identifier: MIT
/**
 * @file utest-lexers-kcmdline.c
 * @brief Implementation of the crinitKernelCmdlineLex() unit test group.
 */

#include "utest-lexers-kcmdline.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitKernelCmdlineLex using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitKernelCmdlineLexTestSuccess),
        cmocka_unit_test(crinitKernelCmdlineLexTestNullInput),
        cmocka_unit_test(crinitKernelCmdlineLexTestLexerError),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
