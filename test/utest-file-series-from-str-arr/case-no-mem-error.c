/**
 * @file case-no-mem-error.c
 * @brief Unit test for EBCL_fileSeriesFromStrArr(), strdup returns NULL.
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

static void EBCL_testVariant(size_t numElements) {
    struct ebcl_FileSeries_t fse;

    char *baseDir = (void *)0xDEADB33F;

    char *strArr[numElements + 1];
    for (size_t i = 0; i < numElements; i++) {
        strArr[i] = (void *)0xd3adda7a;
    }
    strArr[numElements] = NULL;

    print_message("Testing EBCL_fileSeriesFromStrArr with fse = %p, baseDir = %p and strArr = %p.\n", (void *)&fse,
                  (void *)baseDir, (void *)strArr);

    expect_value(__wrap_strdup, s, baseDir);
    will_return(__wrap_strdup, NULL);

    expect_any(__wrap_EBCL_errnoPrintFFL, format);

    assert_int_equal(EBCL_fileSeriesFromStrArr(&fse, baseDir, strArr), -1);
}

void EBCL_fileSeriesFromStrArrTestNoMemError(void **state) {
    EBCL_PARAM_UNUSED(state);

    EBCL_testVariant(0);
    EBCL_testVariant(10);
    EBCL_testVariant(0x1000);
}