/**
 * @file utest-file-series-free-scandir-list.c
 * @brief Implementation of the crinitFreeScandirList() unit test group.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
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
