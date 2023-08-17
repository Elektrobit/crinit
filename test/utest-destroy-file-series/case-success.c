// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitDestroyFileSeries(), successful execution.
 */

#include <stdio.h>

#include "common.h"
#include "fseries.h"
#include "string.h"
#include "unit_test.h"

static void crinitTestVariant(size_t numElements, const char *baseDir) {
    char *fnamesBuff[numElements + 1];
    struct crinitFileSeries_t fse = {.baseDir = (char *)baseDir, .size = numElements};

    if (baseDir) {
        print_message("Testing crinitDestroyFileSeriesTestSuccess with numElement = %ld and baseDir = %s.\n",
                      numElements, baseDir);
    } else {
        print_message("Testing crinitDestroyFileSeriesTestSuccess with numElement = %ld and baseDir = NULL.\n",
                      numElements);
    }

    if (numElements > 0) {
        fnamesBuff[0] = (void *)0xdeadc0de;
        fse.fnames = fnamesBuff;

        expect_value(__wrap_free, ptr, (char *)0xdeadc0de);
        expect_value(__wrap_free, ptr, fse.fnames);
    }

    expect_value(__wrap_free, ptr, baseDir);

    crinitDestroyFileSeries(&fse);

    if (numElements > 0) {
        assert_ptr_equal(fse.fnames, NULL);
        /* HINT: The following test will fail, as the compiler optimizes the fse->fnames[0] = NULL away */
        /* assert_ptr_equal(fnamesBuff[0], NULL); */
    }

    assert_ptr_equal(fse.baseDir, NULL);
    assert_int_equal(fse.size, 0);
}

void crinitDestroyFileSeriesTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *baseDir = "/some/path/to/testdir/";

    crinitTestVariant(0, NULL);
    crinitTestVariant(0, baseDir);
    crinitTestVariant(10, NULL);
    crinitTestVariant(10, baseDir);
}
