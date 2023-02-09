/**
 * @file case-param-null-error.c
 * @brief Unit test for EBCL_fileSeriesFromDir(), given a NULL parameter fse or path.
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
#include "unit_test.h"

void EBCL_fileSeriesFromDirParamNullError(void **state) {
    EBCL_PARAM_UNUSED(state);

    ebcl_FileSeries_t *fse = (void *)0xd3adda7a;
    const char *path = (void *)0xd3adda7a;
    const char *fileSuffix = (void *)0xd3adda7a;

    expect_any_count(__wrap_EBCL_errPrintFFL, format, 8);

    assert_int_equal(EBCL_fileSeriesFromDir(fse, NULL, NULL, false), -1);
    assert_int_equal(EBCL_fileSeriesFromDir(fse, NULL, fileSuffix, false), -1);
    assert_int_equal(EBCL_fileSeriesFromDir(fse, NULL, fileSuffix, true), -1);
    assert_int_equal(EBCL_fileSeriesFromDir(fse, NULL, NULL, true), -1);

    assert_int_equal(EBCL_fileSeriesFromDir(NULL, path, NULL, false), -1);
    assert_int_equal(EBCL_fileSeriesFromDir(NULL, path, fileSuffix, false), -1);
    assert_int_equal(EBCL_fileSeriesFromDir(NULL, path, fileSuffix, true), -1);
    assert_int_equal(EBCL_fileSeriesFromDir(NULL, path, NULL, true), -1);
}
