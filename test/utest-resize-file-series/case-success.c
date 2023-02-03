/**
 * @file case-success.c
 * @brief Unit test for EBCL_resizeFileSeries(), successful execution.
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

static void EBCL_testVariant(size_t initialSize, size_t numElements) {
    char *fnamesBuff[numElements + 1];
    struct ebcl_FileSeries_t fse = {.size = initialSize};

    if (initialSize > 0) {
        fse.fnames = fnamesBuff;
    }

    print_message("Testing EBCL_resizeFileSeriesTestSuccess with %zu elements resizing to %zu elements.\n", initialSize,
                  numElements);

    if (numElements != initialSize) {
        expect_value(__wrap_realloc, ptr, fse.fnames);
        expect_value(__wrap_realloc, size, (numElements + 1) * sizeof(char *));
        will_return(__wrap_realloc, fnamesBuff);
    }

    assert_int_equal(EBCL_resizeFileSeries(&fse, numElements), 0);

    if (numElements > 0 || numElements != initialSize) {
        assert_ptr_equal(fse.fnames, fnamesBuff);
    } else {
        assert_ptr_equal(fse.fnames, NULL);
    }

    assert_int_equal(fse.size, numElements);
}

void EBCL_resizeFileSeriesTestSuccess(void **state) {
    EBCL_PARAM_UNUSED(state);

    size_t maxElements = 0x10000;

    print_message("Testing with %zu max elements.\n", maxElements);

    EBCL_testVariant(0, 0);
    EBCL_testVariant(0, 100);
    EBCL_testVariant(0, maxElements);

    EBCL_testVariant(100, 100);
    EBCL_testVariant(100, maxElements);

    EBCL_testVariant(maxElements, 100);
    EBCL_testVariant(maxElements, maxElements);
}