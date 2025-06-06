// SPDX-License-Identifier: MIT
/**
 * @file case-shrink-zero-error.c
 * @brief Unit test for crinitResizeFileSeries(), successful execution.
 */

#include <stdio.h>

#include "common.h"
#include "fseries.h"
#include "string.h"
#include "unit_test.h"

void crinitResizeFileSeriesTestShrinkZeroError(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitFileSeries_t fse = {.size = 100};

    expect_any(__wrap_crinitErrPrintFFL, format);

    assert_int_equal(crinitResizeFileSeries(&fse, 0), -1);
}
