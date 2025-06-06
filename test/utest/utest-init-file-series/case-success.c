// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitInitFileSeries(), successful execution.
 */

#include <stdio.h>

#include "common.h"
#include "fseries.h"
#include "string.h"
#include "unit_test.h"

static void crinitTestVariant(size_t numElements, const char *baseDir) {
    crinitFileSeries_t fse;
    char *fnamesBuff[numElements + 1];

    if (baseDir) {
        print_message("Testing crinitInitFileSeriesTestSuccess with numElement = %ld and baseDir = %s.\n", numElements,
                      baseDir);
    } else {
        print_message("Testing crinitInitFileSeriesTestSuccess with numElement = %ld and baseDir = NULL.\n",
                      numElements);
    }

    if (baseDir != NULL) {
        expect_value(__wrap_strdup, s, baseDir);
        will_return(__wrap_strdup, baseDir);
    }

    if (numElements > 0) {
        expect_value(__wrap_realloc, ptr, NULL);
        expect_value(__wrap_realloc, size, (numElements + 1) * sizeof(char *));
        will_return(__wrap_realloc, fnamesBuff);
    }

    assert_int_equal(crinitInitFileSeries(&fse, numElements, baseDir), 0);

    if (numElements > 0) {
        assert_ptr_equal(fse.fnames, fnamesBuff);
    } else {
        assert_ptr_equal(fse.fnames, NULL);
    }

    assert_int_equal(fse.size, numElements);
    assert_ptr_equal(fse.baseDir, baseDir);
}

void crinitInitFileSeriesTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *baseDir = "/some/path/to/testdir/";

    crinitTestVariant(0, NULL);
    crinitTestVariant(0, baseDir);
    crinitTestVariant(10, NULL);
    crinitTestVariant(10, baseDir);
}
