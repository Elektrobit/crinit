// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-create-launcher-parameters.h
 * @brief Header declaring the unit tests for crinitCreateLauncherParameters().
 */
#ifndef __UTEST_CREATE_LAUNCHER_PARAMETERS_H__
#define __UTEST_CREATE_LAUNCHER_PARAMETERS_H__

/**
 * Tests successful parsing of a command with one group.
 */
void crinitCfgLauncherCmdHandlerTestWithOneGroupSuccess(void **state);
/**
 * Tests successful parsing of a command with two groups (one main group, one supplementary group).
 */
void crinitCfgLauncherCmdHandlerTestWithTwoGroupsSuccess(void **state);
/**
 * Tests successful parsing of a command with three groups (one main group, two supplementary groups).
 */
void crinitCfgLauncherCmdHandlerTestWithThreeGroupsSuccess(void **state);
#endif /* __UTEST_CREATE_LAUNCHER_PARAMETERS_H__ */
