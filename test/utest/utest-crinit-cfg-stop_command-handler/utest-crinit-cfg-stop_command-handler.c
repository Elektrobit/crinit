// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-cfg-stop_command-handler.c
 * @brief Implementation of the crinitCfgGroupHandler() unit test group.
 */

#include "utest-crinit-cfg-stop_command-handler.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitCfgGroupHandler() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitCfgStopCommandHandlerTestSingleStopCommandSuccess),
        cmocka_unit_test(crinitCfgStopCommandHandlerTestSingleStopCommandWithParameterSuccess),
        cmocka_unit_test(crinitCfgStopCommandHandlerTestNullInput),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
