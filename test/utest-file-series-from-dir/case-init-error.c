/**
 * @file case-init-error.c
 * @brief Unit test for EBCL_fileSeriesFromDir(), given init fails.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>

#include "common.h"
#include "fseries.h"
#include "unit_test.h"

void EBCL_fileSeriesFromDirInitError(void **state) {
    CRINIT_PARAM_UNUSED(state);

    /* Use special pointer to trigger error */
    ebcl_FileSeries_t *fse = (void *)0xbaadda7a;
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

    expect_any(__wrap_crinitErrPrintFFL, format);

    assert_int_equal(EBCL_fileSeriesFromDir(fse, path, NULL, false), -1);
}
