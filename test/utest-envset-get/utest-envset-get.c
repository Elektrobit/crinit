/**
 * @file utest-envset-get.c
 * @brief Implementation of the EBCL_envSetGet() unit test group.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "utest-envset-get.h"

#include "unit_test.h"

/**
 * Runs the unit test group for EBCL_envSetGet() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(EBCL_envSetGetTestSuccess),
        cmocka_unit_test(EBCL_envSetGetTestNullInput),
        cmocka_unit_test(EBCL_envSetGetTestNotFound),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
