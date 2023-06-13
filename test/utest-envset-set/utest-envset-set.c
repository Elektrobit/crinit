/**
 * @file utest-envset-set.c
 * @brief Implementation of the crinitEnvSetSet() unit test group.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "utest-envset-set.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitEnvSetSet() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitEnvSetSetTestSuccess),
        cmocka_unit_test(crinitEnvSetSetTestNullInput),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
