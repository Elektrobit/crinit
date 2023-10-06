// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitFileSeriesFromStrArr(), successful execution.
 */

#include <stdio.h>

#include "common.h"
#include "fseries.h"
#include "string.h"
#include "unit_test.h"

static void crinitTestVariant(size_t numElements) {
    struct crinitFileSeries_t fse;

    const char *baseDir = "/some/path/to/testdir/";
    char *fnamesCopy[numElements + 1];

    print_message("Testing crinitFileSeriesFromStrArr with numElement = %ld and baseDir = %s.\n", numElements, baseDir);

    expect_value(__wrap_calloc, num, numElements + 1);
    expect_value(__wrap_calloc, size, sizeof(char **));
    will_return(__wrap_calloc, fnamesCopy);

    char *fnamesBuff[numElements + 1];
    for (size_t i = 0; i < numElements; i++) {
        fnamesBuff[i] = (void *)0xd3adda7a;
        expect_value(__wrap_strdup, s, fnamesBuff[i]);
        will_return(__wrap_strdup, (void *)0xdeadb33f);
    }
    fnamesBuff[numElements] = NULL;

    expect_value(__wrap_strdup, s, baseDir);
    will_return(__wrap_strdup, baseDir);

    assert_int_equal(crinitFileSeriesFromStrArr(&fse, baseDir, fnamesBuff), 0);

    assert_ptr_equal(fse.fnames, fnamesCopy);
    assert_int_equal(fse.size, numElements);
    assert_ptr_equal(fse.baseDir, baseDir);
}

void crinitFileSeriesFromStrArrTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTestVariant(0);
    crinitTestVariant(10);
    crinitTestVariant(0x1000);
}
