// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-expand-pid-variables.h
 * @brief Header declaring the unit tests for crinitExpandPIDVariablesInSingleCommand().
 */
#ifndef __UTEST_CFG_EXPAND_PID_VARIABLES_H__
#define __UTEST_CFG_EXPAND_PID_VARIABLES_H__

/**
 * Tests successful replacing one variable in a command.
 */
void crinitExpandPIDVariablesInSingleCommandOneVariableReplaced(void **state);

/**
 * Tests successful replacing two variables in a command.
 */
void crinitExpandPIDVariablesInSingleCommandTwoVariablesReplaced(void **state);

/**
 * Tests successful parsing and replacement of a command task structure
 */
void crinitExpandPIDVariablesInCommandsOneVariableInThreeArgv(void **state);

/**
 * Tests successful parsing and replacement of a command task structure
 */
void crinitExpandPIDVariablesInCommandsOneVariableInThreeCommands(void **state);
#endif /* __UTEST_CFG_EXPAND_PID_VARIABLES_H__ */
