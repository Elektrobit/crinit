/**
 * @file utest-file-series-stat-filter.c
 * @brief Implementation of the crinitStatFilter() unit test group.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
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
