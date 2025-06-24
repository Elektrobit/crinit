// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-taskdb-set-task-respawn-inhibit.h
 * @brief Header declaring the unit tests for crinitTaskDBSetTaskRespawnInhibit().
 */
#ifndef __UTEST_TASKDB_SET_TASK_RESPAWN_INHIBIT_H__
#define __UTEST_TASKDB_SET_TASK_RESPAWN_INHIBIT_H__

/**
 * Cleanup function
 */
int crinitTaskDBSetTaskRespawnInhibitTestSuccessTeardown(void **state);

/**
 * Cleanup function
 */
int crinitTaskDBSetTaskRespawnInhibitTestFailureTeardown(void **state);

/**
 * Tests successful execution of crinitTaskDBSetTaskRespawnInhibit
 */
void crinitTaskDBSetTaskRespawnInhibitTestSuccess(void **state);
/**
 * Tests NULL pointer handling on ctx parameter.
 */
void crinitTaskDBSetTaskRespawnInhibitTestCtxNullPointerFailure(void **state);
/**
 * Tests NULL pointer handling on taskName.
 */
void crinitTaskDBSetTaskRespawnInhibitTestTaskNameNullPointerFailure(void **state);
/**
 * Tests error case "task not found"
 */
void crinitTaskDBSetTaskRespawnInhibitTestTaskNotFoundFailure(void **state);

#endif /* __UTEST_TASKDB_SET_TASK_RESPAWN_INHIBIT_H__ */
