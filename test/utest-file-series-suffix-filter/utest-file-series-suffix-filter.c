// SPDX-License-Identifier: MIT
/**
 * @file utest-file-series-suffix-filter.c
 * @brief Implementation of the crinitSuffixFilter() unit test group.
 */

#include "utest-file-series-suffix-filter.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitSuffixFilter using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitSuffixFilterTestSuccess),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
