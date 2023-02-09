/**
 * @file utest-file-series-stat-filter.c
 * @brief Implementation of the EBCL_statFilter() unit test group.
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
 * Runs the unit test group for EBCL_statFilter using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(EBCL_statFilterTestSuccess),
        cmocka_unit_test(EBCL_statFilterTestFstatatFail),
        cmocka_unit_test(EBCL_statFilterTestSisregFail),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
