// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-cfg-stop_command-handler.h
 * @brief Header declaring the unit tests for crinitCfgStopCommandHandler().
 */
#ifndef __UTEST_CFG_STOP_COMMAND_HANDLER_H__
#define __UTEST_CFG_STOP_COMMAND_HANDLER_H__

/**
 * Tests successful parsing of a stop command.
 */
void crinitCfgStopCommandHandlerTestSingleStopCommandSuccess(void **state);
/**
 * Tests successful parsing of a stop command with parameter.
 */
void crinitCfgStopCommandHandlerTestSingleStopCommandWithParameterSuccess(void **state);
/**
 * Tests detection of NULL pointer input.
 */
void crinitCfgStopCommandHandlerTestNullInput(void **state);
#endif /* __UTEST_CFG_STOP_COMMAND_HANDLER_H__ */
