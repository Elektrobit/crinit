/**
 * @file case-no-mem-error.c
 * @brief Unit test for EBCL_resizeFileSeries(), realloc returns NULL.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include <limits.h>
#include <stdio.h>

#include "common.h"
#include "fseries.h"
#include "unit_test.h"

void EBCL_resizeFileSeriesTestNoMemError(void **state) {
    EBCL_PARAM_UNUSED(state);

    struct ebcl_FileSeries_t fse = {
        .fnames = (void *)0xd3adda7a,
    };

    expect_value_count(__wrap_realloc, ptr, fse.fnames, 2);
    expect_any_count(__wrap_realloc, size, 2);
    will_return_count(__wrap_realloc, NULL, 2);

    expect_any_count(__wrap_EBCL_errnoPrintFFL, format, 2);

    assert_int_equal(EBCL_resizeFileSeries(&fse, 100), -1);

    /* fse.fnames should not be changed on error */
    assert_ptr_not_equal(fse.fnames, NULL);

    assert_int_equal(EBCL_resizeFileSeries(&fse, SIZE_MAX), -1);
}
