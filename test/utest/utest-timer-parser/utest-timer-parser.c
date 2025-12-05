// SPDX-License-Identifier: MIT
/**
 * @file utest-lexers-envset-outer.c
 * @brief Implementation of the crinitEnvsetOuterLex() unit test group.
 */

#include "utest-timer-parser.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitEnvVarOuterLex using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitTimerParserTestSuccess),
        cmocka_unit_test(crinitTimerParserTestError),
        cmocka_unit_test(crinitTimerParserTestNullParam),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
