/**
 * @file utest-lexers-envset-outer.c
 * @brief Implementation of the EBCL_envsetOuterLex() unit test group.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
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
