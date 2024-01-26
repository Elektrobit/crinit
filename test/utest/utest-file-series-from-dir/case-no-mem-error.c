// SPDX-License-Identifier: MIT
/**
 * @file case-no-mem-error.c
 * @brief Unit test for crinitFileSeriesFromDir(), given malloc fails.
 */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "common.h"
#include "fseries.h"
#include "unit_test.h"

#define DUMMY_FILENAME "dummy.file"

void crinitFileSeriesFromDirNoMemError(void **state) {
    CRINIT_PARAM_UNUSED(state);

    /* Initialize a dummy file series structure */
    char *fnamesBuff[1];
    crinitFileSeries_t fse = {.fnames = fnamesBuff, .size = crinitNumElements(fnamesBuff)};
    const char *path = (void *)0xd3adda7a;
    const DIR *dptr = (void *)0xd3adda7a;

    struct dirent **scanList = NULL;
    scanList = calloc(1, sizeof(*scanList));
    assert_ptr_not_equal(scanList, NULL);
    scanList[0] = calloc(1, sizeof(*scanList[0]));
    assert_ptr_not_equal(scanList[0], NULL);
    strcpy(scanList[0]->d_name, DUMMY_FILENAME);

    expect_value(__wrap_opendir, name, path);
    will_return(__wrap_opendir, dptr);

    expect_value(__wrap_dirfd, dirp, dptr);
    will_return(__wrap_dirfd, 13);

    expect_value(__wrap_scandir, dirp, path);
    expect_any(__wrap_scandir, namelist);
    will_set_parameter(__wrap_scandir, namelist, scanList);
    expect_any(__wrap_scandir, filter);
    expect_any(__wrap_scandir, compar);
    will_return(__wrap_scandir, crinitNumElements(fnamesBuff));

    expect_value(__wrap_closedir, dirp, dptr);

    expect_value(__wrap_strlen, s, scanList[0]->d_name);
    will_return(__wrap_strlen, sizeof(DUMMY_FILENAME) - 1);

    expect_value(__wrap_malloc, size, sizeof(DUMMY_FILENAME));
    will_return(__wrap_malloc, NULL);

    expect_any(__wrap_crinitErrnoPrintFFL, format);

    assert_int_equal(crinitFileSeriesFromDir(&fse, path, NULL, false), -1);

    free(scanList[0]);
    free(scanList);
}
