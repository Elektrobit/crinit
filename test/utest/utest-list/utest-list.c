// SPDX-License-Identifier: MIT
/**
 * @file utest-list.c
 * @brief Implementation of the crinitList_t library unit test group.
 */

#include "utest-list.h"

#include "unit_test.h"

/**
 * Runs the unit test group for the crinitList_t library functions using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitListTestSuccess),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
