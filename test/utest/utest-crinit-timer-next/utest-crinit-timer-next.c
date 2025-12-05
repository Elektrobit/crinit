// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-task-create-from-conf-kvlist.c
 * @brief Implementation of the crinitCfgUserHandler() unit test group.
 */

#include "utest-crinit-timer-next.h"

#include <time.h>

#include "common.h"
#include "unit_test.h"

/**
 * Runs the unit test group for crinitCfgUserHandler() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitTimerNextTimeFail),
        cmocka_unit_test(crinitTimerNextTimeRangeSuccess),
        cmocka_unit_test(crinitTimerNextTimeThuJan11970),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
