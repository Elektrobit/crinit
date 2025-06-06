// SPDX-License-Identifier: MIT
/**
 * @file case-no-mem-error.c
 * @brief Unit test for crinitInitFileSeries(), strdup returns NULL.
 */

#include <stdio.h>

#include "common.h"
#include "fseries.h"
#include "unit_test.h"

void crinitInitFileSeriesTestNoMemError(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitFileSeries_t fse;
    char *baseDir = (void *)0xDEADB33F;

    expect_value(__wrap_strdup, s, baseDir);
    will_return(__wrap_strdup, NULL);

    expect_any(__wrap_crinitErrnoPrintFFL, format);

    assert_int_equal(crinitInitFileSeries(&fse, 0, baseDir), -1);
}
