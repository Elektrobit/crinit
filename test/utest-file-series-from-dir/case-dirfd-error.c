// SPDX-License-Identifier: MIT
/**
 * @file case-dirfd-error.c
 * @brief Unit test for crinitFileSeriesFromDir(), given dirfd fails.
 */

#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>

#include "common.h"
#include "fseries.h"
#include "unit_test.h"

void crinitFileSeriesFromDirDirfdError(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitFileSeries_t *fse = (void *)0xd3adda7a;
    const char *path = (void *)0xd3adda7a;
    const DIR *dptr = (void *)0xd3adda7a;

    expect_value(__wrap_opendir, name, path);
    will_return(__wrap_opendir, dptr);

    expect_value(__wrap_dirfd, dirp, dptr);
    will_return(__wrap_dirfd, -1);

    expect_any(__wrap_crinitErrnoPrintFFL, format);

    expect_value(__wrap_closedir, dirp, dptr);

    assert_int_equal(crinitFileSeriesFromDir(fse, path, NULL, false), -1);
}
