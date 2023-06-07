/**
 * @file utest-cfgmap-order.c
 * @brief Implementation of the unit test group for the order/size of EBCL_cfgMap.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "utest-cfgmap-order.h"

#include "unit_test.h"

/**
 * Runs the unit test group for EBCL_crinitSetVerbose using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {cmocka_unit_test(EBCL_cfgMapRegressionTest)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
