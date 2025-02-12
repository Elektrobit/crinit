// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-cfg-launch-cmd-handler.c
 * @brief Implementation of the crinitCfgLauncherCmdHandler() unit test group.
 */

#include "utest-crinit-cfg-launch-cmd-handler.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitCfgLauncherCmdHandler() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitCfgLauncherCmdHandlerTestExistingExecutableSuccess),
        cmocka_unit_test(crinitCfgLauncherCmdDefaultValue),
        cmocka_unit_test(crinitCfgLauncherCmdHandlerTestExistingFileNotExecutable),
        cmocka_unit_test(crinitCfgLauncherCmdHandlerTestNullInput),
        cmocka_unit_test(crinitCfgLauncherCmdHandlerTestEmptyInput),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
