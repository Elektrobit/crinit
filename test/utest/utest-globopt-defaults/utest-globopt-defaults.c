// SPDX-License-Identifier: MIT
/**
 * @file utest-globopt-defaults.c
 * @brief Implementation of the test group for the regression test for default initialization of global options.
 */

#include "utest-globopt-defaults.h"

#include "unit_test.h"

/**
 * Runs the regression test for default initialization of global options using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {cmocka_unit_test(crinitGlobDefRegressionTest)};
    return cmocka_run_group_tests(tests, NULL, NULL);
}
