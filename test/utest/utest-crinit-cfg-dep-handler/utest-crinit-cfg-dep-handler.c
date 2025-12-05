// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-cfg-elos-poll-interval-handler.c
 * @brief Implementation of the crinitCfgElosPollIntervalHandler() unit test group.
 */

#include "utest-crinit-cfg-dep-handler.h"

#include <stdlib.h>

#include "task.h"
#include "unit_test.h"

int crinitTestSetup(void **state) {
    *state = calloc(1, sizeof(crinitTask_t));
    return 0;
}
int crinitTestTeardown(void **state) {
    crinitTask_t *tgt = *state;
    crinitFreeTask(tgt);
    tgt = NULL;
    return 0;
}
/**
 * Runs the unit test group for crinitCfgLauncherCmdHandler() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(crinitCfgDepHandlerTestSuccess, crinitTestSetup, crinitTestTeardown),
        cmocka_unit_test_setup_teardown(crinitCfgDepHandlerTestErrConfigType, crinitTestSetup, crinitTestTeardown),
        cmocka_unit_test_setup_teardown(crinitCfgDepHandlerTestInvalidValue, crinitTestSetup, crinitTestTeardown),
        cmocka_unit_test_setup_teardown(crinitCfgDepHandlerTestNullInput, crinitTestSetup, crinitTestTeardown),
        cmocka_unit_test_setup_teardown(crinitCfgDepHandlerTestEmptyInput, crinitTestSetup, crinitTestTeardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
