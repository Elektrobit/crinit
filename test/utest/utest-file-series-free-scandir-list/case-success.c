// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitFreeScandirList(), successful execution.
 */

#include <stdio.h>

#include "common.h"
#include "fseries.h"
#include "mock-free.h"
#include "unit_test.h"
#include "utest-file-series-free-scandir-list.h"

static void crinitTestVariant(int size) {
    struct dirent *list[size];
    struct dirent **scanList;
    if (size > 0) {
        print_message("Testing crinitFreeScandirList with %d entries.\n", size);
        scanList = list;

        for (int i = 0; i < size; i++) {
            list[i] = (void *)0xd3adda7a;
        }

        expect_value_count(__wrap_free, ptr, (void *)0xd3adda7a, size);
        expect_value(__wrap_free, ptr, (void *)list);
    } else if (size == 0) {
        print_message("Testing crinitFreeScandirList with empty scan list.\n");
        scanList = list;

        expect_value(__wrap_free, ptr, list);
    } else {
        print_message("Testing crinitFreeScandirList with NULL scan list.\n");
        scanList = NULL;
        size = 0;
    }

    crinitMockFreeEnabled = true;
    crinitFreeScandirList(scanList, size);
    crinitMockFreeEnabled = false;
}

void crinitFreeScandirListTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTestVariant(-1);
    crinitTestVariant(0);
    crinitTestVariant(10);
    crinitTestVariant(0x1000);
}
