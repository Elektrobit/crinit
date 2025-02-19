// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-cfg-user-handler.c
 * @brief Implementation of the crinitCfgUserHandler() unit test group.
 */

#include "utest-crinit-cfg-user-handler.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitCfgUserHandler() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitCfgUserHandlerTestNumericSuccess),
        cmocka_unit_test(crinitCfgUserHandlerTestAlphaInputSuccess),
        cmocka_unit_test(crinitCfgUserHandlerTestNegativeInput),
        cmocka_unit_test(crinitCfgUserHandlerTestNullInput),
        cmocka_unit_test(crinitCfgUserHandlerTestEmptyInput),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
