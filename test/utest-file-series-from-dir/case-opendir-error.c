/**
 * @file case-fse-null-error.c
 * @brief Unit test for EBCL_fileSeriesFromDir(), given opendir fails.
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

void EBCL_fileSeriesFromDirOpendirError(void **state) {
    EBCL_PARAM_UNUSED(state);

    ebcl_FileSeries_t *fse = (void *)0xd3adda7a;
    const char *path = (void *)0xd3adda7a;

    expect_value(__wrap_opendir, name, path);
    will_return(__wrap_opendir, NULL);

    expect_any(__wrap_EBCL_errnoPrintFFL, format);

    assert_int_equal(EBCL_fileSeriesFromDir(fse, path, NULL, false), -1);
}