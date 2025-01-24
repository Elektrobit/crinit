// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-exec-rtim-cmd.c
 * @brief Implementation of the crinitExecRtimCmd() unit test group.
 */

#include "utest-crinit-exec-rtim-cmd.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitExecRtimCmd() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitExecRtimCmdTestShutdownWithStopCommand),
        cmocka_unit_test(crinitExecRtimCmdTestShutdownWithTwoTasksWithStopCommand),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
