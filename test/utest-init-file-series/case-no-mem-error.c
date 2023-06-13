/**
 * @file case-no-mem-error.c
 * @brief Unit test for crinitInitFileSeries(), strdup returns NULL.
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

void crinitInitFileSeriesTestNoMemError(void **state) {
    CRINIT_PARAM_UNUSED(state);

    struct crinitFileSeries_t fse;
    char *baseDir = (void *)0xDEADB33F;

    expect_value(__wrap_strdup, s, baseDir);
    will_return(__wrap_strdup, NULL);

    expect_any(__wrap_crinitErrnoPrintFFL, format);

    assert_int_equal(crinitInitFileSeries(&fse, 0, baseDir), -1);
}
