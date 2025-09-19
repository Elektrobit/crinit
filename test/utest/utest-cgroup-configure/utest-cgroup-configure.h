// SPDX-License-Identifier: MIT
/**
 * @file utest-cgroup-configure.h
 * @brief Header declaring the unit tests for crinitCGroupAssignPID().
 */
#ifdef ENABLE_CGROUP
#ifndef __UTEST_CGROUP_ASSIGN_PID_H__
#define __UTEST_CGROUP_ASSIGN_PID_H__

/**
 * Unit test for crinitCGroupAssignPID(), successful execution.
 */
void crinitCGroupConfigureTestSuccess(void **state);
void crinitCGroupConfigureTestSuccessParent(void **state);

/**
 * Unit test for crinitCGroupAssignPID(), parameter error.
 */
void crinitCGroupConfigureTestWrongInput(void **state);

/**
 * Unit test for crinitCGroupAssignPID(), open error.
 */
void crinitCGroupConfigureTestOpenFail(void **state);

/**
 * Unit test for crinitCGroupAssignPID(), openat error.
 */
void crinitCGroupConfigureTestOpenatFailFirst(void **state);
void crinitCGroupConfigureTestOpenatFailSecond(void **state);
void crinitCGroupConfigureTestOpenatFailThird(void **state);

/**
 * Unit test for crinitCGroupAssignPID(), writev error.
 */
void crinitCGroupConfigureTestWritevFail(void **state);

/**
 * Unit test for crinitCGroupAssignPID(), mkdir error.
 */
void crinitCGroupConfigureTestMkdirFail(void **state);

#endif /* __UTEST_CGROUP_ASSIGN_PID_H__ */
#endif
