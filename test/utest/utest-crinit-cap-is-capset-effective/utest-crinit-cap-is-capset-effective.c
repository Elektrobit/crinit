// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-cap-is-capset-effective.c
 * @brief Implementation of the test_crinitCapIsCapsetEffective() unit test group.
 */

#include "utest-crinit-cap-is-capset-effective.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitCfgUserHandler() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_crinitCapIsCapsetEffective_lsb_low),
        cmocka_unit_test(test_crinitCapIsCapsetEffective_msb_low),
        cmocka_unit_test(test_crinitCapIsCapsetEffective_lsb_high),
        cmocka_unit_test(test_crinitCapIsCapsetEffective_last_supported),
        cmocka_unit_test(test_crinitCapIsCapsetEffective_not_set),
        cmocka_unit_test(test_crinitCapIsCapsetEffective_unsupported_capability),
        cmocka_unit_test(test_crinitCapIsCapsetEffective_first_after_last_capability),
        cmocka_unit_test(test_crinitCapIsCapsetEffective_last_possible_capability),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
