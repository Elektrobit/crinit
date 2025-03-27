// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-create-launcher-parameters.c
 * @brief Implementation of the crinitCreateLauncherParameters() unit test group.
 */

#include "utest-crinit-create-launcher-parameters.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitCreateLauncherParameters() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitCfgLauncherCmdHandlerTestWithOneGroupSuccess),
        cmocka_unit_test(crinitCfgLauncherCmdHandlerTestWithTwoGroupsSuccess),
        cmocka_unit_test(crinitCfgLauncherCmdHandlerTestWithThreeGroupsSuccess),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
