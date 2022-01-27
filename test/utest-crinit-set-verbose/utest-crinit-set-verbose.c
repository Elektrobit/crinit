/**
 * @file utest-crinit-set-verbose.c
 * @brief Implementation of the EBCL_crinitSetVerbose() unit test group.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "utest-crinit-set-verbose.h"

#include <stdio.h>
#include <stdlib.h>

#include "unit_test.h"

/**
 * Runs the unit test group for EBCL_crinitSetVerbose using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {cmocka_unit_test(EBCL_crinitSetVerboseTestSuccess),
                                       cmocka_unit_test(EBCL_crinitSetVerboseTestGlobOptError)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
