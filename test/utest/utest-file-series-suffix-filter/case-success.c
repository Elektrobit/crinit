// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitSuffixFilter(), successful execution.
 */

#include <stdio.h>

#include "common.h"
#include "fseries.h"
#include "unit_test.h"
#include "utest-file-series-suffix-filter.h"

static void crinitTestVariant(const char *path, const char *fileSuffix, bool res) {
    if (fileSuffix && fileSuffix[0] != '\0') {
        print_message("Testing crinitSuffixFilter with path '%s' and file suffix '%s'.\n", path, fileSuffix);
    } else {
        print_message("Testing crinitSuffixFilter with path '%s' and without file suffix.\n", path);
    }

    assert_int_equal(crinitSuffixFilter(path, fileSuffix), res);
}

void crinitSuffixFilterTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    // clang-format off
    // Rationale: unreadable output of clang-format
    const char *path[] = {
        "",
        "file", "file.gz", "file.odt", "file.xml", "file.json", "file.tar.gz",
        "/abs/path/to/file", "/abs/path/to/file.gz", "/abs/path/to/file.odt", "/abs/path/to/file.xml", "/abs/path/to/file.json", "/abs/path/to/file.tar.gz",
        "./path/to/dir/file", "./path/to/dir/file.gz", "./path/to/dir/file.odt", "./path/to/dir/file.xml", "./path/to/dir/file.json", "./path/to/dir/file.tar.gz",
    };
    const char *fileSuffix[] = {
        "", "gz", "odt", "xml", "json", "tar.gz", NULL
    };
    const bool res[] = {
        /* empty path */
        1, 0, 0, 0, 0, 0, 1,
        /* file name only */
        1, 0, 0, 0, 0, 0, 1,
        1, 1, 0, 0, 0, 0, 1,
        1, 0, 1, 0, 0, 0, 1,
        1, 0, 0, 1, 0, 0, 1,
        1, 0, 0, 0, 1, 0, 1,
        1, 1, 0, 0, 0, 1, 1,
        /* absolute file path */
        1, 0, 0, 0, 0, 0, 1,
        1, 1, 0, 0, 0, 0, 1,
        1, 0, 1, 0, 0, 0, 1,
        1, 0, 0, 1, 0, 0, 1,
        1, 0, 0, 0, 1, 0, 1,
        1, 1, 0, 0, 0, 1, 1,
        /* relative file path */
        1, 0, 0, 0, 0, 0, 1,
        1, 1, 0, 0, 0, 0, 1,
        1, 0, 1, 0, 0, 0, 1,
        1, 0, 0, 1, 0, 0, 1,
        1, 0, 0, 0, 1, 0, 1,
        1, 1, 0, 0, 0, 1, 1,
    };
    // clang-format on

    const int pathSize = ARRAY_SIZE(path);
    const int fileSuffixSize = ARRAY_SIZE(fileSuffix);
    for (int i = 0; i < pathSize; i++) {
        for (int j = 0; j < fileSuffixSize; j++) {
            crinitTestVariant(path[i], fileSuffix[j], res[i * fileSuffixSize + j]);
        }
    }
}
