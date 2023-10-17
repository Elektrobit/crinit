// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitStatFilter(), successful execution.
 */

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>

#include "common.h"
#include "fseries.h"
#include "unit_test.h"
#include "utest-file-series-stat-filter.h"

static void crinitTestVariant(const char *path, int baseDirFd, bool followLinks) {
    struct stat buf = {.st_mode = __S_IFREG};

    if (followLinks) {
        print_message("Testing crinitStatFilter with path '%s', dir file descriptor '%d', following links.\n", path,
                      baseDirFd);
    } else {
        print_message("Testing crinitStatFilter with path '%s', dir file descriptor '%d', without following links.\n",
                      path, baseDirFd);
    }

    expect_value(__wrap_fstatat, fd, baseDirFd);
    expect_value(__wrap_fstatat, path, path);
    expect_any(__wrap_fstatat, buf);
    will_set_parameter(__wrap_fstatat, buf, &buf);
    expect_value(__wrap_fstatat, flag, followLinks ? 0 : AT_SYMLINK_NOFOLLOW);
    will_return(__wrap_fstatat, 0);

    assert_int_not_equal(crinitStatFilter(path, baseDirFd, followLinks), 0);
}

void crinitStatFilterTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    int baseDirFd[] = {0, 10, 100};
    const bool followLinks[] = {true, false};

    const char *path[] = {
        "file", "file.gz", "/abs/path/to/file", "/abs/path/to/file.gz", "./path/to/dir/file", "./path/to/dir/file.gz",
    };
    const int pathSize = ARRAY_SIZE(path);
    const int baseDirFdSize = ARRAY_SIZE(baseDirFd);
    const int followLinksSize = ARRAY_SIZE(followLinks);
    for (int i = 0; i < pathSize; i++) {
        for (int j = 0; j < baseDirFdSize; j++) {
            for (int k = 0; k < followLinksSize; k++) {
                crinitTestVariant(path[i], baseDirFd[j], followLinks[k]);
            }
        }
    }
}
