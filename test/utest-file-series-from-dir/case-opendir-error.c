/**
 * @file case-opendir-error.c
 * @brief Unit test for crinitFileSeriesFromDir(), given opendir fails.
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

void crinitFileSeriesFromDirOpendirError(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitFileSeries_t *fse = (void *)0xd3adda7a;
    const char *path = (void *)0xd3adda7a;

    expect_value(__wrap_opendir, name, path);
    will_return(__wrap_opendir, NULL);

    expect_any(__wrap_crinitErrnoPrintFFL, format);

    assert_int_equal(crinitFileSeriesFromDir(fse, path, NULL, false), -1);
}
