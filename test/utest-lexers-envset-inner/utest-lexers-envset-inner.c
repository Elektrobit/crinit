/**
 * @file utest-lexers-envset-inner.c
 * @brief Implementation of the EBCL_envsetInnerLex() unit test group.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "utest-lexers-envset-inner.h"

#include "unit_test.h"

/**
 * Runs the unit test group for EBCL_envVarInnerLex using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(EBCL_envVarInnerLexTestSuccess),
        cmocka_unit_test(EBCL_envVarInnerLexTestNullInput),
        cmocka_unit_test(EBCL_envVarInnerLexTestLexerError),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
