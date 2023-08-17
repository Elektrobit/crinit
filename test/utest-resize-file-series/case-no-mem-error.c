// SPDX-License-Identifier: MIT
/**
 * @file case-no-mem-error.c
 * @brief Unit test for crinitResizeFileSeries(), realloc returns NULL.
 */

#include <limits.h>
#include <stdio.h>

#include "common.h"
#include "fseries.h"
#include "unit_test.h"

void crinitResizeFileSeriesTestNoMemError(void **state) {
    CRINIT_PARAM_UNUSED(state);

    struct crinitFileSeries_t fse = {
        .fnames = (void *)0xd3adda7a,
    };

    expect_value_count(__wrap_realloc, ptr, fse.fnames, 2);
    expect_any_count(__wrap_realloc, size, 2);
    will_return_count(__wrap_realloc, NULL, 2);

    expect_any_count(__wrap_crinitErrnoPrintFFL, format, 2);

    assert_int_equal(crinitResizeFileSeries(&fse, 100), -1);

    /* fse.fnames should not be changed on error */
    assert_ptr_not_equal(fse.fnames, NULL);

    assert_int_equal(crinitResizeFileSeries(&fse, SIZE_MAX), -1);
}
