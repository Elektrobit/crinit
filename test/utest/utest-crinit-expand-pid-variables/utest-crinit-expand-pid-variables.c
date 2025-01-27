// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-expand-pid-variables.c
 * @brief Implementation of the crinitExpandPIDVariablesInSingleCommand() unit test group.
 */

#include "utest-crinit-expand-pid-variables.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitExpandPIDVariablesInSingleCommand() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitExpandPIDVariablesInSingleCommandOneVariableReplaced),
        cmocka_unit_test(crinitExpandPIDVariablesInSingleCommandTwoVariablesReplaced),
        cmocka_unit_test(crinitExpandPIDVariablesInCommandsOneVariableInThreeArgv),
        cmocka_unit_test(crinitExpandPIDVariablesInCommandsOneVariableInThreeCommands),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
