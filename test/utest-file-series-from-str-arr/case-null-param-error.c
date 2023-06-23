/**
 * @file case-null-param-error.c
 * @brief Unit test for crinitFileSeriesFromStrArr(), given NULL parameter.
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

static void crinitTestVariant(crinitFileSeries_t *fse, const char *baseDir, char **strArr) {
    print_message("Testing crinitFileSeriesFromStrArr with fse = %p, baseDir = %p and strArr = %p.\n", (void *)fse,
                  (void *)baseDir, (void *)strArr);

    expect_any(__wrap_crinitErrPrintFFL, format);

    assert_int_equal(crinitFileSeriesFromStrArr(fse, baseDir, strArr), -1);
}

void crinitFileSeriesFromStrArrTestNullParamError(void **state) {
    CRINIT_PARAM_UNUSED(state);

    struct crinitFileSeries_t fse;
    const char *baseDir = (void *)0xdeadda7a;
    char **strArr = (void *)0xbaadda7a;

    crinitTestVariant(NULL, NULL, NULL);
    crinitTestVariant(&fse, NULL, NULL);
    crinitTestVariant(&fse, baseDir, NULL);
    crinitTestVariant(&fse, NULL, strArr);
    crinitTestVariant(NULL, baseDir, NULL);
    crinitTestVariant(NULL, baseDir, strArr);
    crinitTestVariant(NULL, NULL, strArr);
}
