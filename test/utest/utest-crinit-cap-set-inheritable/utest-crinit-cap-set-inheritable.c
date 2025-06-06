// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-cap-set-inheritable.c
 * @brief Implementation of the crinitCapSetInheritable() unit test group.
 */

#include "utest-crinit-cap-set-inheritable.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitCfgUserHandler() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_crinitCapSetInheritable),
        cmocka_unit_test(test_crinitCapSetInheritable_last_supported),
        cmocka_unit_test(test_crinitCapSetInheritable_invalid_capability),
        cmocka_unit_test(test_crinitCapSetInheritable_invalid_capability_range),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
