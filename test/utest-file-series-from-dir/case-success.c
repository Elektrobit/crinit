/**
 * @file case-success.c
 * @brief Unit test for EBCL_fileSeriesFromDir(), successful execution.
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
#include "string.h"
#include "unit_test.h"

static void EBCL_testVariant(size_t numElements, const char *path, const char *fileSuffix, bool followLinks) {
    const DIR *dptr = (void *)0xd3adda7a;
    const int dfp = 0xc0ff;

    struct dirent *scandirList[numElements];
    for (size_t i = 0; i < numElements; i++) {
        scandirList[i] = (void *)0xd3adda7a;
    }

    char *runnerPtr = (void *)0xbaadc0ff;
    char *fnamesRoot = (void *)0xdeadc0ff;
    char *fnamesBuff[numElements + 1];
    for (size_t i = 0; i < numElements; i++) {
        fnamesBuff[i] = (void *)0xd3adda7a;
    }
    fnamesBuff[numElements] = NULL;

    struct ebcl_FileSeries_t fse = {.size = numElements, .fnames = fnamesBuff};

    if (followLinks) {
        print_message(
            "Testing EBCL_fileSeriesFromDir with numElement = %ld, path = %s, fileSuffix = %s and following "
            "symlinks.\n",
            numElements, path, fileSuffix);
    } else {
        print_message(
            "Testing EBCL_fileSeriesFromDir with numElement = %ld, path = %s, fileSuffix = %s and NOT following "
            "symlinks.\n",
            numElements, path, fileSuffix);
    }

    expect_value(__wrap_opendir, name, path);
    will_return(__wrap_opendir, dptr);

    expect_value(__wrap_dirfd, dirp, dptr);
    will_return(__wrap_dirfd, dfp);

    expect_value(__wrap_scandir, dirp, path);
    expect_any(__wrap_scandir, namelist);
    will_set_parameter(__wrap_scandir, namelist, scandirList);
    expect_any(__wrap_scandir, filter);
    expect_any(__wrap_scandir, compar);
    will_return(__wrap_scandir, dfp);

    expect_value(__wrap_closedir, dirp, dptr);

    expect_any_count(__wrap_strlen, s, numElements);
    will_return_count(__wrap_strlen, 0, numElements);

    expect_value(__wrap_malloc, size, numElements);
    will_return(__wrap_malloc, fnamesRoot);

    expect_any_count(__wrap_stpcpy, dest, numElements);
    expect_any_count(__wrap_stpcpy, src, numElements);
    will_return_count(__wrap_stpcpy, runnerPtr, numElements);

    assert_int_equal(EBCL_fileSeriesFromDir(&fse, path, fileSuffix, followLinks), 0);
}

void EBCL_fileSeriesFromDirTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    // clang-format off
    const bool followLinks[] = {true, false};
    const char *path[] = {
        "file", "file.gz", "file.odt", "file.xml", "file.json", "file.tar.gz",
        "/abs/path/to/file", "/abs/path/to/file.gz", "/abs/path/to/file.odt", "/abs/path/to/file.xml", "/abs/path/to/file.json", "/abs/path/to/file.tar.gz",
        "./path/to/dir/file", "./path/to/dir/file.gz", "./path/to/dir/file.odt", "./path/to/dir/file.xml", "./path/to/dir/file.json", "./path/to/dir/file.tar.gz"
    };
    const char *fileSuffix[] = {
        "", "gz", "odt", "xml", "json", "tar.gz"
    };
    // clang-format on

    const int pathSize = ARRAY_SIZE(path);
    const int fileSuffixSize = ARRAY_SIZE(fileSuffix);
    const int followLinksSize = ARRAY_SIZE(followLinks);
    for (int i = 0; i < pathSize; i++) {
        for (int j = 0; j < fileSuffixSize; j++) {
            for (int k = 0; k < followLinksSize; k++) {
                EBCL_testVariant(10, path[i], fileSuffix[j], followLinks[k]);
            }
        }
    }
}
