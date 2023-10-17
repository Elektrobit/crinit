// SPDX-License-Identifier: MIT
/**
 * @file utest-envset-dup.c
 * @brief Implementation of the crinitEnvSetDup() unit test group.
 */

#include "utest-envset-dup.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitEnvSetDup() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitEnvSetDupTestSuccess),
        cmocka_unit_test(crinitEnvSetDupTestNullInput),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
