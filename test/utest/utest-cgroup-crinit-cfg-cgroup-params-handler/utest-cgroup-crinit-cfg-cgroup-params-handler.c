// SPDX-License-Identifier: MIT
/**
 * @file utest-cgroup-crinit-cfg-cgroup-params-handler.c
 * @brief Implementation of the crinitCfgCGroupParamsHandler() unit test group.
 */

#include "utest-cgroup-crinit-cfg-cgroup-params-handler.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitCfgCGroupParamsHandler() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitCfgCGroupParamsHandlerTestSingleKeyValueSuccess),
        cmocka_unit_test(crinitCfgCGroupParamsHandlerTestTwoKeyValueSuccess),
        cmocka_unit_test(crinitCfgCGroupParamsHandlerTestNullInput),
        cmocka_unit_test(crinitCfgCGroupParamsHandlerTestEmptyInput),
        cmocka_unit_test(crinitCfgCGroupParamsHandlerTestInvalidInputMissingDelimiter),
        cmocka_unit_test(crinitCfgCGroupParamsHandlerTestInvalidInputMissingCGroupName),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
