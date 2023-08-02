// SPDX-License-Identifier: MIT
/**
 * @file utest-lexers-envset-inner.c
 * @brief Implementation of the crinitEnvSetInnerLex() unit test group.
 */

#include "utest-lexers-envset-inner.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitEnvVarInnerLex using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitEnvVarInnerLexTestSuccess),
        cmocka_unit_test(crinitEnvVarInnerLexTestNullInput),
        cmocka_unit_test(crinitEnvVarInnerLexTestLexerError),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
