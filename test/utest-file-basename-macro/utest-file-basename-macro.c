// SPDX-License-Identifier: MIT
/**
 * @file utest-file-basename-macro.c
 * @brief Implementation of the unit test group for the __FILE_BASENAME__ macro.
 */

#include "utest-file-basename-macro.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitClientSetVerbose using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {cmocka_unit_test(crinitFileBasenameMacroRegressionTest)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
