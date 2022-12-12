/**
 * @file utest-envset-destroy.c
 * @brief Implementation of the EBCL_envSetDestroy() unit test group.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "utest-envset-destroy.h"

#include "unit_test.h"

/**
 * Runs the unit test group for EBCL_envVarInnerLex using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(EBCL_envSetDestroyTestSuccess),
        cmocka_unit_test(EBCL_envSetDestroyTestNullInput),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
