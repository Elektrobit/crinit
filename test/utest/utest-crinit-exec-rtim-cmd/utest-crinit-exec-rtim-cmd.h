// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-exec-rtim-cmd.h
 * @brief Header declaring the unit tests for crinitCfgStopCommandHandler().
 */
#ifndef __UTEST_EXEC_RTIM_CMD_H__
#define __UTEST_EXEC_RTIM_CMD_H__

/**
 * Tests successful execution of a stop command on system shutdown
 */
void crinitExecRtimCmdTestShutdownWithStopCommand(void **state);

/**
 * Tests successful execution of two task with a stop command on system shutdown
 */
void crinitExecRtimCmdTestShutdownWithTwoTasksWithStopCommand(void **state);
#endif /* __UTEST_EXEC_RTIM_CMD_H__ */
