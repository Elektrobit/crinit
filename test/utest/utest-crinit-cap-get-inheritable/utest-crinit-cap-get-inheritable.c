// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-cap-get-inheritable.c
 * @brief Implementation of the crinitCapGetInheritable() unit test group.
 */

#include "utest-crinit-cap-get-inheritable.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitCapGetInheritable() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_crinitCapGetInheritable),
        cmocka_unit_test(test_crinitCapGetInheritable_resultParmInitialized),
        cmocka_unit_test(test_crinitCapGetInheritable_invalid_capability_pointer),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
