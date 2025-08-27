// SPDX-License-Identifier: MIT
/**
 * @file utest-cgroup-assign-pid.c
 * @brief Implementation of the crinitCgroupAssignPID() unit test group.
 */

#include "utest-cgroup-assign-pid.h"

#include <sys/types.h>

#include "common.h"
#include "unit_test.h"

/**
 * Runs the unit test group for crinitCgroupAssignPID using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        // clang-format off
        // Rationale: unreadable output of clang-format
        cmocka_unit_test(crinitCgroupAssignPIDTestSuccess),
        cmocka_unit_test(crinitCgroupAssignPIDTestSuccessParent),
        cmocka_unit_test(crinitCgroupAssignPIDTestWrongInput),
        cmocka_unit_test(crinitCgroupAssignPIDTestOpenFail),
        cmocka_unit_test(crinitCgroupAssignPIDTestOpenatFailFirst),
        cmocka_unit_test(crinitCgroupAssignPIDTestOpenatFailSecond),
        cmocka_unit_test(crinitCgroupAssignPIDTestOpenatFailThird),
        cmocka_unit_test(crinitCgroupAssignPIDTestWritevFail),
        // clang-format on
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
