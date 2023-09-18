// SPDX-License-Identifier: MIT
/**
 * @file utest-envset-destroy.c
 * @brief Implementation of the crinitEnvSetDestroy() unit test group.
 */

#include "utest-envset-destroy.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitEnvVarInnerLex using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitEnvSetDestroyTestSuccess),
        cmocka_unit_test(crinitEnvSetDestroyTestNullInput),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
