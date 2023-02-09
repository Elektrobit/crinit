/**
 * @file case-fse-null-error.c
 * @brief Unit test for EBCL_fileSeriesFromStrArr(), given NULL parameter.
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

static void EBCL_testVariant(ebcl_FileSeries_t *fse, const char *baseDir, char **strArr) {
    print_message("Testing EBCL_fileSeriesFromStrArr with fse = %p, baseDir = %p and strArr = %p.\n", (void *)fse,
                  (void *)baseDir, (void *)strArr);

    expect_any(__wrap_EBCL_errPrintFFL, format);

    assert_int_equal(EBCL_fileSeriesFromStrArr(fse, baseDir, strArr), -1);
}

void EBCL_fileSeriesFromStrArrTestNullParamError(void **state) {
    EBCL_PARAM_UNUSED(state);

    struct ebcl_FileSeries_t fse;
    const char *baseDir = (void *)0xdeadda7a;
    char **strArr = (void *)0xbaadda7a;

    EBCL_testVariant(NULL, NULL, NULL);
    EBCL_testVariant(&fse, NULL, NULL);
    EBCL_testVariant(&fse, baseDir, NULL);
    EBCL_testVariant(&fse, NULL, strArr);
    EBCL_testVariant(NULL, baseDir, NULL);
    EBCL_testVariant(NULL, baseDir, strArr);
    EBCL_testVariant(NULL, NULL, strArr);
}
