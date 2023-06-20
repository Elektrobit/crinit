/**
 * @file utest-init-file-series.c
 * @brief Implementation of the crinitInitFileSeries() unit test group.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "utest-init-file-series.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitInitFileSeries using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitInitFileSeriesTestSuccess),
        cmocka_unit_test(crinitInitFileSeriesTestNoMemError),
        cmocka_unit_test(crinitInitFileSeriesTestFseNullError),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
