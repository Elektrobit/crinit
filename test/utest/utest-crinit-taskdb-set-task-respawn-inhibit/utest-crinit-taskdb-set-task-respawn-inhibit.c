// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-taskdb-set-task-respawn-inhibit.c
 * @brief Implementation of crinitTaskDBSetTaskRespawnInhibit()
 */

#include "utest-crinit-taskdb-set-task-respawn-inhibit.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitTaskDBSetTaskRespawnInhibit() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_teardown(crinitTaskDBSetTaskRespawnInhibitTestSuccess,
                                  crinitTaskDBSetTaskRespawnInhibitTestSuccessTeardown),
        cmocka_unit_test(crinitTaskDBSetTaskRespawnInhibitTestCtxNullPointerFailure),
        cmocka_unit_test_teardown(crinitTaskDBSetTaskRespawnInhibitTestTaskNameNullPointerFailure,
                                  crinitTaskDBSetTaskRespawnInhibitTestFailureTeardown),
        cmocka_unit_test_teardown(crinitTaskDBSetTaskRespawnInhibitTestTaskNotFoundFailure,
                                  crinitTaskDBSetTaskRespawnInhibitTestFailureTeardown)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
