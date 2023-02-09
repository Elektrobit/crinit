/**
 * @file case-fse-null-error.c
 * @brief Unit test for EBCL_resizeFileSeries(), given file series is NULL.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include <limits.h>
#include <stdio.h>

#include "common.h"
#include "fseries.h"
#include "unit_test.h"

void EBCL_resizeFileSeriesTestFseNullError(void **state) {
    EBCL_PARAM_UNUSED(state);

    expect_any_count(__wrap_EBCL_errPrintFFL, format, 3);

    assert_int_equal(EBCL_resizeFileSeries(NULL, 0), -1);
    assert_int_equal(EBCL_resizeFileSeries(NULL, 100), -1);
    assert_int_equal(EBCL_resizeFileSeries(NULL, SIZE_MAX), -1);
}
