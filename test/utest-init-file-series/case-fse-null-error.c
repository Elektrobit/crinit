/**
 * @file case-fse-null-error.c
 * @brief Unit test for crinitEnvSetDestroy(), given file series is NULL.
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
#include "unit_test.h"

void EBCL_initFileSeriesTestFseNullError(void **state) {
    CRINIT_PARAM_UNUSED(state);

    expect_any(__wrap_crinitErrPrintFFL, format);

    assert_int_equal(EBCL_initFileSeries(NULL, 0, NULL), -1);
}
