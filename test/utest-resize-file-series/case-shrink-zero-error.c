/**
 * @file case-shrink-zero-error.c
 * @brief Unit test for crinitResizeFileSeries(), successful execution.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include <stdio.h>

#include "common.h"
#include "fseries.h"
#include "string.h"
#include "unit_test.h"

void crinitResizeFileSeriesTestShrinkZeroError(void **state) {
    CRINIT_PARAM_UNUSED(state);

    struct crinitFileSeries_t fse = {.size = 100};

    expect_any(__wrap_crinitErrPrintFFL, format);

    assert_int_equal(crinitResizeFileSeries(&fse, 0), -1);
}
