// SPDX-License-Identifier: MIT
/**
 * @file utest-destroy-file-series.c
 * @brief Implementation of the crinitDestroyFileSeries() unit test group.
 */

#include "utest-destroy-file-series.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitDestroyFileSeries using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitDestroyFileSeriesTestSuccess),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
