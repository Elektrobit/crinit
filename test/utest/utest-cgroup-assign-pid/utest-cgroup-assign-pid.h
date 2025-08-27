// SPDX-License-Identifier: MIT
/**
 * @file utest-cgroup-assign-pid.h
 * @brief Header declaring the unit tests for crinitCGroupAssignPID().
 */
#ifndef __UTEST_CGROUP_ASSIGN_PID_H__
#define __UTEST_CGROUP_ASSIGN_PID_H__

/**
 * Unit test for crinitCGroupAssignPID(), successful execution.
 */
void crinitCgroupAssignPIDTestSuccess(void **state);
void crinitCgroupAssignPIDTestSuccessParent(void **state);

/**
 * Unit test for crinitCGroupAssignPID(), parameter error.
 */
void crinitCgroupAssignPIDTestWrongInput(void **state);

/**
 * Unit test for crinitCGroupAssignPID(), open error.
 */
void crinitCgroupAssignPIDTestOpenFail(void **state);

/**
 * Unit test for crinitCGroupAssignPID(), openat error.
 */
void crinitCgroupAssignPIDTestOpenatFailFirst(void **state);
void crinitCgroupAssignPIDTestOpenatFailSecond(void **state);
void crinitCgroupAssignPIDTestOpenatFailThird(void **state);

/**
 * Unit test for crinitCGroupAssignPID(), writev error.
 */
void crinitCgroupAssignPIDTestWritevFail(void **state);

#endif /* __UTEST_CGROUP_ASSIGN_PID_H__ */
