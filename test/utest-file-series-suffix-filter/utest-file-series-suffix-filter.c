/**
 * @file utest-file-series-suffix-filter.c
 * @brief Implementation of the EBCL_suffixFilter() unit test group.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "utest-file-series-suffix-filter.h"

#include "unit_test.h"

/**
 * Runs the unit test group for EBCL_suffixFilter using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(EBCL_suffixFilterTestSuccess),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
