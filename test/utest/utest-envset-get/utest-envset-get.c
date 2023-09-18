// SPDX-License-Identifier: MIT
/**
 * @file utest-envset-get.c
 * @brief Implementation of the crinitEnvSetGet() unit test group.
 */

#include "utest-envset-get.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitEnvSetGet() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitEnvSetGetTestSuccess),
        cmocka_unit_test(crinitEnvSetGetTestNullInput),
        cmocka_unit_test(crinitEnvSetGetTestNotFound),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
