/**
 * @file utest-resize-file-series.c
 * @brief Implementation of the EBCL_resizeFileSeries() unit test group.
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
 * Runs the unit test group for EBCL_resizeFileSeries using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(EBCL_resizeFileSeriesTestSuccess),
        cmocka_unit_test(EBCL_resizeFileSeriesTestNoMemError),
        cmocka_unit_test(EBCL_resizeFileSeriesTestFseNullError),
        cmocka_unit_test(EBCL_resizeFileSeriesTestShrinkZeroError),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
