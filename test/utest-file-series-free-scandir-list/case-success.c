/**
 * @file case-success.c
 * @brief Unit test for EBCL_freeScandirList(), successful execution.
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
#include "unit_test.h"
#include "utest-file-series-free-scandir-list.h"

static void EBCL_testVariant(int size) {
    struct dirent *list[size];
    struct dirent **scanList;
    if (size > 0) {
        print_message("Testing EBCL_freeScandirList with %d entries.\n", size);
        scanList = list;

        for (int i = 0; i < size; i++) {
            list[i] = (void *)0xd3adda7a;
        }

        expect_value_count(__wrap_free, ptr, (void *)0xd3adda7a, size);
        expect_value(__wrap_free, ptr, (void *)list);
    } else if (size == 0) {
        print_message("Testing EBCL_freeScandirList with empty scan list.\n");
        scanList = list;

        expect_value(__wrap_free, ptr, list);
    } else {
        print_message("Testing EBCL_freeScandirList with NULL scan list.\n");
        scanList = NULL;
        size = 0;
    }

    EBCL_freeScandirList(scanList, size);
}

void EBCL_freeScandirListTestSuccess(void **state) {
    EBCL_PARAM_UNUSED(state);

    EBCL_testVariant(-1);
    EBCL_testVariant(0);
    EBCL_testVariant(10);
    EBCL_testVariant(0x1000);
}
