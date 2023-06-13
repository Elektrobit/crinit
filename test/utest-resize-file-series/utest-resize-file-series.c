/**
 * @file utest-resize-file-series.c
 * @brief Implementation of the crinitResizeFileSeries() unit test group.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "utest-resize-file-series.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitResizeFileSeries using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitResizeFileSeriesTestSuccess),
        cmocka_unit_test(crinitResizeFileSeriesTestNoMemError),
        cmocka_unit_test(crinitResizeFileSeriesTestFseNullError),
        cmocka_unit_test(crinitResizeFileSeriesTestShrinkZeroError),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
