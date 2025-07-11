// SPDX-License-Identifier: MIT
/**
 * @file utest-cgroup-crinit-cfg-cgroup-name-handler.h
 * @brief Header declaring the unit tests for crinitCfgCGroupNameHandler().
 */
#ifndef __UTEST_CGROUP_CRINIT_CFG_CGROUP_NAME_HANDLER_H__
#define __UTEST_CGROUP_CRINIT_CFG_CGROUP_NAME_HANDLER_H__

/**
 * Tests successful parsing of a alphabetical group name (e.g. "sshd.cg") instead of an ID.
 */
void crinitCfgGroupHandlerTestAlphaInputSuccess(void **state);
/**
 * Tests detection of NULL pointer input.
 */
void crinitCfgGroupHandlerTestNullInput(void **state);
/**
 * Tests handling of empty value part.
 */
void crinitCfgGroupHandlerTestEmptyInput(void **state);
#endif /* __UTEST_CGROUP_CRINIT_CFG_CGROUP_NAME_HANDLER_H__ */
