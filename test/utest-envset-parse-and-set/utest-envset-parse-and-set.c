/**
 * @file utest-envset-parse-and-set.c
 * @brief Implementation of the EBCL_envSetParseAndSet() unit test group.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "utest-envset-parse-and-set.h"

#include "unit_test.h"

/**
 * Runs the unit test group for EBCL_envSetParseAndSet() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(EBCL_envSetParseAndSetTestSuccess),
        cmocka_unit_test(EBCL_envSetParseAndSetTestWrongInput),
        cmocka_unit_test(EBCL_envSetParseAndSetTestNullInput),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
