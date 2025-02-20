// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-cfg-group-handler.c
 * @brief Implementation of the crinitCfgGroupHandler() unit test group.
 */

#include "utest-crinit-cfg-group-handler.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitCfgGroupHandler() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitCfgGroupHandlerTestNumericSuccess),
        cmocka_unit_test(crinitCfgGroupHandlerTestAlphaInputSuccess),
        cmocka_unit_test(crinitCfgGroupHandlerTestNegativeInput),
        cmocka_unit_test(crinitCfgGroupHandlerTestNullInput),
        cmocka_unit_test(crinitCfgGroupHandlerTestEmptyInput),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
