// SPDX-License-Identifier: MIT
/**
 * @file utest-envset-init.c
 * @brief Implementation of the crinitEnvSetInit() unit test group.
 */

#include "utest-envset-init.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitEnvVarInnerLex using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitEnvSetInitTestSuccess),
        cmocka_unit_test(crinitEnvSetInitTestNullInput),
        cmocka_unit_test(crinitEnvSetInitTestMallocError),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
