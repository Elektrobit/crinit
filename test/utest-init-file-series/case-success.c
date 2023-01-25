/**
 * @file case-success.c
 * @brief Unit test for EBCL_initFileSeries(), successful execution.
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
#include "string.h"
#include "unit_test.h"

static void EBCL_testVariant(size_t numElements, const char *baseDir) {
    struct ebcl_FileSeries_t fse;
    char *fnamesBuff[numElements + 1];

    if (baseDir) {
        print_message("Testing EBCL_initFileSeriesTestSuccess with numElement = %ld and baseDir = %s.\n", numElements,
                      baseDir);
    } else {
        print_message("Testing EBCL_initFileSeriesTestSuccess with numElement = %ld and baseDir = NULL.\n",
                      numElements);
    }

    if (baseDir != NULL) {
        expect_value(__wrap_strdup, s, baseDir);
        will_return(__wrap_strdup, baseDir);
    }

    if (numElements > 0) {
        expect_value(__wrap_realloc, ptr, NULL);
        expect_value(__wrap_realloc, size, (numElements + 1) * sizeof(char *));
        will_return(__wrap_realloc, fnamesBuff);
    }

    assert_int_equal(EBCL_initFileSeries(&fse, numElements, baseDir), 0);

    if (numElements > 0) {
        assert_ptr_equal(fse.fnames, fnamesBuff);
    } else {
        assert_ptr_equal(fse.fnames, NULL);
    }

    assert_int_equal(fse.size, numElements);
    assert_ptr_equal(fse.baseDir, baseDir);
}

void EBCL_initFileSeriesTestSuccess(void **state) {
    EBCL_PARAM_UNUSED(state);

    const char *baseDir = "/some/path/to/testdir/";

    EBCL_testVariant(0, NULL);
    EBCL_testVariant(0, baseDir);
    EBCL_testVariant(10, NULL);
    EBCL_testVariant(10, baseDir);
}