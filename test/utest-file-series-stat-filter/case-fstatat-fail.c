// SPDX-License-Identifier: MIT
/**
 * @file case-fstatat-fail.c
 * @brief Unit test for crinitStatFilter(), fstatat fail.
 */

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>

#include "common.h"
#include "fseries.h"
#include "unit_test.h"
#include "utest-file-series-stat-filter.h"

void crinitStatFilterTestFstatatFail(void **state) {
    CRINIT_PARAM_UNUSED(state);
    struct stat buf = {0};

    int baseDirFd = 0xd3adda7a;
    const char *path = "file.gz";

    expect_value_count(__wrap_fstatat, fd, baseDirFd, 2);
    expect_value_count(__wrap_fstatat, path, path, 2);
    expect_any_count(__wrap_fstatat, buf, 2);
    expect_value(__wrap_fstatat, flag, AT_SYMLINK_NOFOLLOW);
    expect_value(__wrap_fstatat, flag, 0);
    will_set_parameter(__wrap_fstatat, buf, &buf);
    will_return(__wrap_fstatat, 0);

    assert_int_equal(crinitStatFilter(path, baseDirFd, false), 0);

    will_set_parameter(__wrap_fstatat, buf, &buf);
    will_return(__wrap_fstatat, 0);
    assert_int_equal(crinitStatFilter(path, baseDirFd, true), 0);
}
