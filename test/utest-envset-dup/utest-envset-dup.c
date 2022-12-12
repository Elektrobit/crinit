/**
 * @file utest-envset-dup.c
 * @brief Implementation of the EBCL_envSetDup() unit test group.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "utest-envset-dup.h"

#include "unit_test.h"

/**
 * Runs the unit test group for EBCL_envSetDup() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(EBCL_envSetDupTestSuccess),
        cmocka_unit_test(EBCL_envSetDupTestNullInput),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
