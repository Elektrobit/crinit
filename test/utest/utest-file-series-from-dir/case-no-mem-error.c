// SPDX-License-Identifier: MIT
/**
 * @file case-no-mem-error.c
 * @brief Unit test for crinitFileSeriesFromDir(), given malloc fails.
 */

#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>

#include "common.h"
#include "fseries.h"
#include "unit_test.h"

void crinitFileSeriesFromDirNoMemError(void **state) {
    CRINIT_PARAM_UNUSED(state);

    /* Use special pointer to trigger error */
    char *fnamesBuff[1];
    crinitFileSeries_t fse = {.fnames = fnamesBuff};
    const char *path = (void *)0xd3adda7a;
    const DIR *dptr = (void *)0xd3adda7a;

    struct dirent **scanList = NULL;

    expect_value(__wrap_opendir, name, path);
    will_return(__wrap_opendir, dptr);

    expect_value(__wrap_dirfd, dirp, dptr);
    will_return(__wrap_dirfd, 13);

    expect_value(__wrap_scandir, dirp, path);
    expect_any(__wrap_scandir, namelist);
    will_set_parameter(__wrap_scandir, namelist, scanList);
    expect_any(__wrap_scandir, filter);
    expect_any(__wrap_scandir, compar);
    will_return(__wrap_scandir, 0);

    expect_value(__wrap_closedir, dirp, dptr);

    expect_value(__wrap_malloc, size, 0);
    will_return(__wrap_malloc, NULL);

    expect_any(__wrap_crinitErrnoPrintFFL, format);

    assert_int_equal(crinitFileSeriesFromDir(&fse, path, NULL, false), -1);
}
