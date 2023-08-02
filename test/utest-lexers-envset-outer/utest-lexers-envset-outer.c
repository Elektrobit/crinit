// SPDX-License-Identifier: MIT
/**
 * @file utest-lexers-envset-outer.c
 * @brief Implementation of the crinitEnvsetOuterLex() unit test group.
 */

#include "utest-lexers-envset-outer.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitEnvVarOuterLex using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitEnvVarOuterLexTestSuccess),
        cmocka_unit_test(crinitEnvVarOuterLexTestNullInput),
        cmocka_unit_test(crinitEnvVarOuterLexTestLexerError),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
