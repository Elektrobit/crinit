// SPDX-License-Identifier: MIT
/**
 * @file case-fse-null-error.c
 * @brief Unit test for crinitResizeFileSeries(), given file series is NULL.
 */

#include <limits.h>
#include <stdio.h>

#include "common.h"
#include "fseries.h"
#include "unit_test.h"

void crinitResizeFileSeriesTestFseNullError(void **state) {
    CRINIT_PARAM_UNUSED(state);

    expect_any_count(__wrap_crinitErrPrintFFL, format, 3);

    assert_int_equal(crinitResizeFileSeries(NULL, 0), -1);
    assert_int_equal(crinitResizeFileSeries(NULL, 100), -1);
    assert_int_equal(crinitResizeFileSeries(NULL, SIZE_MAX), -1);
}
