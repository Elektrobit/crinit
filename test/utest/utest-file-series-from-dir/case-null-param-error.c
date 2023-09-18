// SPDX-License-Identifier: MIT
/**
 * @file case-null-param-error.c
 * @brief Unit test for crinitFileSeriesFromDir(), given a NULL parameter fse or path.
 */

#include <stdio.h>

#include "common.h"
#include "fseries.h"
#include "unit_test.h"

void crinitFileSeriesFromDirParamNullError(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitFileSeries_t *fse = (void *)0xd3adda7a;
    const char *path = (void *)0xd3adda7a;
    const char *fileSuffix = (void *)0xd3adda7a;

    expect_any_count(__wrap_crinitErrPrintFFL, format, 8);

    assert_int_equal(crinitFileSeriesFromDir(fse, NULL, NULL, false), -1);
    assert_int_equal(crinitFileSeriesFromDir(fse, NULL, fileSuffix, false), -1);
    assert_int_equal(crinitFileSeriesFromDir(fse, NULL, fileSuffix, true), -1);
    assert_int_equal(crinitFileSeriesFromDir(fse, NULL, NULL, true), -1);

    assert_int_equal(crinitFileSeriesFromDir(NULL, path, NULL, false), -1);
    assert_int_equal(crinitFileSeriesFromDir(NULL, path, fileSuffix, false), -1);
    assert_int_equal(crinitFileSeriesFromDir(NULL, path, fileSuffix, true), -1);
    assert_int_equal(crinitFileSeriesFromDir(NULL, path, NULL, true), -1);
}
