/**
 * @file utest-file-series-from-str-arr.c
 * @brief Implementation of the crinitFileSeriesFromStrArr() unit test group.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
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
