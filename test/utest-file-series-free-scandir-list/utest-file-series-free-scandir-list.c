// SPDX-License-Identifier: MIT
/**
 * @file utest-file-series-free-scandir-list.c
 * @brief Implementation of the crinitFreeScandirList() unit test group.
 */

#include "utest-file-series-free-scandir-list.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitFreeScandirList using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitFreeScandirListTestSuccess),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
