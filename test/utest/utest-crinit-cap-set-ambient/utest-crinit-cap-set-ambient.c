// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-cap-set-ambient.c
 * @brief Implementation of the crinitCapSetAmbient() unit test group.
 */

#include "utest-crinit-cap-set-ambient.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitCfgUserHandler() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_crinitCapSetAmbient_single_capability_set),
        cmocka_unit_test(test_crinitCapSetAmbient_multiple_capability_set),
        cmocka_unit_test(test_crinitCapSetAmbient_last_capability_set),
        cmocka_unit_test(test_crinitCapSetAmbient_no_capability_set),
        cmocka_unit_test(test_crinitCapSetAmbient_invalid_capability_typo),
        cmocka_unit_test(test_crinitCapSetAmbient_invalid_capability_range),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
