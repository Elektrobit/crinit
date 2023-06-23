/**
 * @file case-success.c
 * @brief Unit test for crinitFileSeriesFromStrArr(), successful execution.
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

static void crinitTestVariant(size_t numElements) {
    struct crinitFileSeries_t fse;

    const char *baseDir = "/some/path/to/testdir/";

    char *fnamesBuff[numElements + 1];
    for (size_t i = 0; i < numElements; i++) {
        fnamesBuff[i] = (void *)0xd3adda7a;
    }
    fnamesBuff[numElements] = NULL;

    print_message("Testing crinitFileSeriesFromStrArr with numElement = %ld and baseDir = %s.\n", numElements, baseDir);

    expect_value(__wrap_strdup, s, baseDir);
    will_return(__wrap_strdup, baseDir);

    assert_int_equal(crinitFileSeriesFromStrArr(&fse, baseDir, fnamesBuff), 0);

    assert_ptr_equal(fse.fnames, fnamesBuff);
    assert_int_equal(fse.size, numElements);
    assert_ptr_equal(fse.baseDir, baseDir);
}

void crinitFileSeriesFromStrArrTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTestVariant(0);
    crinitTestVariant(10);
    crinitTestVariant(0x1000);
}
