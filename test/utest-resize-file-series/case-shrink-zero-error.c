/**
 * @file case-shrink-zero-error.c
 * @brief Unit test for EBCL_resizeFileSeries(), successful execution.
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

void EBCL_resizeFileSeriesTestShrinkZeroError(void **state) {
    EBCL_PARAM_UNUSED(state);
    
    struct ebcl_FileSeries_t fse = {
        .size = 100
    };

    expect_any(__wrap_EBCL_errPrintFFL, format);

    assert_int_equal(EBCL_resizeFileSeries(&fse, 0), -1);
}