// SPDX-License-Identifier: MIT
/**
 * @file case-fse-null-error.c
 * @brief Unit test for crinitEnvSetDestroy(), given file series is NULL.
 */

#include <stdio.h>

#include "common.h"
#include "fseries.h"
#include "unit_test.h"

void crinitInitFileSeriesTestFseNullError(void **state) {
    CRINIT_PARAM_UNUSED(state);

    expect_any(__wrap_crinitErrPrintFFL, format);

    assert_int_equal(crinitInitFileSeries(NULL, 0, NULL), -1);
}
