// SPDX-License-Identifier: MIT
/**
 * @file case-wrong-input.c
 * @brief Unit test for crinitConfConvToEnvSetMember(), handling of invalid string input.
 */

#ifdef ENABLE_CGROUP
#include "cgroup.h"
#include "common.h"
#include "unit_test.h"

void crinitCgroupAssignPIDTestWrongInput(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitCgroup_t validCgroup = {0};
    validCgroup.name = "myCgroup";
    crinitCgroup_t invalidCgroup = {0};
    pid_t validPID = 1;
    pid_t invalidPID = -1;

    assert_int_equal(crinitCGroupAssignPID(NULL, validPID), -1);
    assert_int_equal(crinitCGroupAssignPID(&validCgroup, invalidPID), -1);
    assert_int_equal(crinitCGroupAssignPID(&invalidCgroup, validPID), -1);
}
#endif
