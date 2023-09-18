// SPDX-License-Identifier: MIT
/**
 * @file utest-envset-set.c
 * @brief Implementation of the crinitEnvSetSet() unit test group.
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
