/**
 * @file case-success.c
 * @brief Unit test for EBCL_fileSeriesFromStrArr(), successful execution.
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

static void EBCL_testVariant(size_t numElements) {
    struct ebcl_FileSeries_t fse;

    const char *baseDir = "/some/path/to/testdir/";

    char *fnamesBuff[numElements + 1];
    for (size_t i = 0; i < numElements; i++) {
        fnamesBuff[i] = (void *)0xd3adda7a;
    }
    fnamesBuff[numElements] = NULL;

    print_message("Testing EBCL_fileSeriesFromStrArr with numElement = %ld and baseDir = %s.\n", numElements, baseDir);

    expect_value(__wrap_strdup, s, baseDir);
    will_return(__wrap_strdup, baseDir);

    assert_int_equal(EBCL_fileSeriesFromStrArr(&fse, baseDir, fnamesBuff), 0);

    assert_ptr_equal(fse.fnames, fnamesBuff);
    assert_int_equal(fse.size, numElements);
    assert_ptr_equal(fse.baseDir, baseDir);
}

void EBCL_fileSeriesFromStrArrTestSuccess(void **state) {
    EBCL_PARAM_UNUSED(state);

    EBCL_testVariant(0);
    EBCL_testVariant(10);
    EBCL_testVariant(0x1000);
}