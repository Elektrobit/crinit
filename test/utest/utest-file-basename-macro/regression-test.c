// SPDX-License-Identifier: MIT
/**
 * @file regression-test.c
 * @brief Implementation of a regression test to check the __FILE_BASENAME__ macro.
 */
#include <libgen.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "logio.h"
#include "unit_test.h"
#include "utest-file-basename-macro.h"

void crinitFileBasenameMacroRegressionTest(void **state) {
    CRINIT_PARAM_UNUSED(state);

    // Check that __FILE_BASENAME__ is indeed the basename of __FILE__
    // We need a working copy as POSIX does not guarantee that basename does not modify its argument.
    char *filepath = strdup(__FILE__);
    assert_ptr_not_equal(filepath, NULL);
    assert_int_equal(strcmp(basename(filepath), __FILE_BASENAME__), 0);
    free(filepath);
}
