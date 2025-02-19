// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-cfg-launch-cmd-handler.h
 * @brief Header declaring the unit tests for crinitCfgLauncherCmdHandler().
 */
#ifndef __UTEST_CFG_LAUNCH_CMD_HANDLER_H__
#define __UTEST_CFG_LAUNCH_CMD_HANDLER_H__

/**
 * Tests successful parsing of an existing executable.
 */
void crinitCfgLauncherCmdHandlerTestExistingExecutableSuccess(void **state);
/**
 * Tests default value.
 */
void crinitCfgLauncherCmdDefaultValue(void **state);
/**
 * Tests unsuccessful parsing of an existing file that is no executable.
 */
void crinitCfgLauncherCmdHandlerTestExistingFileNotExecutable(void **state);
/**
 * Tests detection of NULL pointer input.
 */
void crinitCfgLauncherCmdHandlerTestNullInput(void **state);
/**
 * Tests handling of empty value part.
 */
void crinitCfgLauncherCmdHandlerTestEmptyInput(void **state);
#endif /* __UTEST_CFG_LAUNCH_CMD_HANDLER_H__ */
