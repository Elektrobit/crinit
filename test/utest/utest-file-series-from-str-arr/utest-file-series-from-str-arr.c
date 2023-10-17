// SPDX-License-Identifier: MIT
/**
 * @file utest-file-series-from-str-arr.c
 * @brief Implementation of the crinitFileSeriesFromStrArr() unit test group.
 */

#include "utest-file-series-from-str-arr.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitFileSeriesFromStrArr using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitFileSeriesFromStrArrTestSuccess),
        cmocka_unit_test(crinitFileSeriesFromStrArrTestNoMemError),
        cmocka_unit_test(crinitFileSeriesFromStrArrTestNullParamError),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
