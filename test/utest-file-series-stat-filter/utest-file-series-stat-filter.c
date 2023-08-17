// SPDX-License-Identifier: MIT
/**
 * @file utest-file-series-stat-filter.c
 * @brief Implementation of the crinitStatFilter() unit test group.
 */

#include "utest-file-series-stat-filter.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitStatFilter using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitStatFilterTestSuccess),
        cmocka_unit_test(crinitStatFilterTestFstatatFail),
        cmocka_unit_test(crinitStatFilterTestSisregFail),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
