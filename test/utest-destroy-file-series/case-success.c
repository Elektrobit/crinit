/**
 * @file case-success.c
 * @brief Unit test for EBCL_destroyFileSeries(), successful execution.
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
    char *fnamesBuff[numElements + 1];
    struct ebcl_FileSeries_t fse = {.baseDir = (char *)baseDir, .size = numElements};

    if (baseDir) {
        print_message("Testing EBCL_destroyFileSeriesTestSuccess with numElement = %ld and baseDir = %s.\n",
                      numElements, baseDir);
    } else {
        print_message("Testing EBCL_destroyFileSeriesTestSuccess with numElement = %ld and baseDir = NULL.\n",
                      numElements);
    }

    if (numElements > 0) {
        fnamesBuff[0] = (void *)0xdeadc0de;
        fse.fnames = fnamesBuff;

        expect_value(__wrap_free, ptr, (char *)0xdeadc0de);
        expect_value(__wrap_free, ptr, fse.fnames);
    }

    expect_value(__wrap_free, ptr, baseDir);

    EBCL_destroyFileSeries(&fse);

    if (numElements > 0) {
        assert_ptr_equal(fse.fnames, NULL);
        /* HINT: The following test will fail, as the compiler optimizes the fse->fnames[0] = NULL away */
        /* assert_ptr_equal(fnamesBuff[0], NULL); */
    }

    assert_ptr_equal(fse.baseDir, NULL);
    assert_int_equal(fse.size, 0);
}

void EBCL_destroyFileSeriesTestSuccess(void **state) {
    EBCL_PARAM_UNUSED(state);

    const char *baseDir = "/some/path/to/testdir/";

    EBCL_testVariant(0, NULL);
    EBCL_testVariant(0, baseDir);
    EBCL_testVariant(10, NULL);
    EBCL_testVariant(10, baseDir);
}