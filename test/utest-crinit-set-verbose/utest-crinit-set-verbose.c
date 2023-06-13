/**
 * @file utest-crinit-set-verbose.c
 * @brief Implementation of the crinitClientSetVerbose() unit test group.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "utest-crinit-set-verbose.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitClientSetVerbose using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {cmocka_unit_test(crinitClientSetVerboseTestSuccess),
                                       cmocka_unit_test(crinitClientSetVerboseTestGlobOptError)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
