// SPDX-License-Identifier: MIT
/**
 * @file utest-inih-configuration.c
 * @brief Implementation of the unit test group for the libinih compile-time configuration.
 */

#include "utest-inih-configuration.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitClientSetVerbose using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {cmocka_unit_test(crinitInihConfigurationRegressionTest)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
