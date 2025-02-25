// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-cfg-elos-poll-interval-handler.c
 * @brief Implementation of the crinitCfgElosPollIntervalHandler() unit test group.
 */

#include "utest-crinit-cfg-elos-poll-interval-handler.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitCfgLauncherCmdHandler() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitCfgElosPollIntervalHandlerTestRuntimeSettingSuccess),
        cmocka_unit_test(crinitCfgElosPollIntervalDefaultValue),
        cmocka_unit_test(crinitCfgElosPollIntervalHandlerTestInvalidInput),
        cmocka_unit_test(crinitCfgElosPollIntervalHandlerTestNullInput),
        cmocka_unit_test(crinitCfgElosPollIntervalHandlerTestEmptyInput),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
