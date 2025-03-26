// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitResizeFileSeries(), successful execution.
 */

#include <stdio.h>

#include "common.h"
#include "fseries.h"
#include "string.h"
#include "unit_test.h"

static void crinitTestVariant(size_t initialSize, size_t numElements) {
    char *fnamesBuff[numElements + 1];
    crinitFileSeries_t fse = {.size = initialSize};

    if (initialSize > 0) {
        fse.fnames = fnamesBuff;
    }

    print_message("Testing crinitResizeFileSeriesTestSuccess with %zu elements resizing to %zu elements.\n",
                  initialSize, numElements);

    if (numElements != initialSize) {
        expect_value(__wrap_realloc, ptr, fse.fnames);
        expect_value(__wrap_realloc, size, (numElements + 1) * sizeof(char *));
        will_return(__wrap_realloc, fnamesBuff);
    }

    assert_int_equal(crinitResizeFileSeries(&fse, numElements), 0);

    if (numElements > 0 || numElements != initialSize) {
        assert_ptr_equal(fse.fnames, fnamesBuff);
    } else {
        assert_ptr_equal(fse.fnames, NULL);
    }

    assert_int_equal(fse.size, numElements);
}

void crinitResizeFileSeriesTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    size_t maxElements = 0x10000;

    print_message("Testing with %zu max elements.\n", maxElements);

    crinitTestVariant(0, 0);
    crinitTestVariant(0, 100);
    crinitTestVariant(0, maxElements);

    crinitTestVariant(100, 100);
    crinitTestVariant(100, maxElements);

    crinitTestVariant(maxElements, 100);
    crinitTestVariant(maxElements, maxElements);
}
