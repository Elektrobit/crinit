// SPDX-License-Identifier: MIT
/**
 * @file utest-cfgmap-order.c
 * @brief Implementation of the unit test group for the order and completeness of crinit{Task,Series}CfgMap.
 */

#include "utest-cfgmap-order.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitClientSetVerbose using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {cmocka_unit_test(crinitCfgMapRegressionTest)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
