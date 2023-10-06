// SPDX-License-Identifier: MIT
/**
 * @file case-no-mem-error.c
 * @brief Unit test for crinitFileSeriesFromStrArr(), strdup returns NULL.
 */

#include <stdio.h>

#include "common.h"
#include "fseries.h"
#include "unit_test.h"

static void crinitTestVariant(size_t numElements) {
    struct crinitFileSeries_t fse;

    char *baseDir = (void *)0xDEADB33F;
    char *fnamesCopy[numElements + 1];

    expect_value(__wrap_calloc, num, numElements + 1);
    expect_value(__wrap_calloc, size, sizeof(char **));
    will_return(__wrap_calloc, fnamesCopy);

    char *strArr[numElements + 1];
    for (size_t i = 0; i < numElements; i++) {
        strArr[i] = (void *)0xd3adda7a;
        expect_value(__wrap_strdup, s, strArr[i]);
        will_return(__wrap_strdup, (void *)0xd3adda7a);
    }
    strArr[numElements] = NULL;

    print_message("Testing crinitFileSeriesFromStrArr with fse = %p, baseDir = %p and strArr = %p.\n", (void *)&fse,
                  (void *)baseDir, (void *)strArr);

    expect_value(__wrap_strdup, s, baseDir);
    will_return(__wrap_strdup, NULL);

    expect_any(__wrap_crinitErrnoPrintFFL, format);

    assert_int_equal(crinitFileSeriesFromStrArr(&fse, baseDir, strArr), -1);
}

void crinitFileSeriesFromStrArrTestNoMemError(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTestVariant(0);
    crinitTestVariant(10);
    crinitTestVariant(0x1000);
}
