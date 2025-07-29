// SPDX-License-Identifier: MIT
/**
 * @file utest-cgroup-crinit-root-name-handler.h
 * @brief Header declaring the unit tests for crinitCfgCgroupRootNameHandler(().
 */
#ifndef __UTEST_CGROUP_CRINIT_CFG_CGROUP_ROOT_NAME_HANDLER_H__
#define __UTEST_CGROUP_CRINIT_CFG_CGROUP_ROOT_NAME_HANDLER_H__

/**
 * Tests successful parsing of a alphabetical group name (e.g. "sshd.cg") instead of an ID.
 */
void crinitCfgCgroupRootNameHandlerTestAlphaInputSuccess(void **state);
/**
 * Tests detection of NULL pointer input.
 */
void crinitCfgCgroupRootNameHandlerTestNullInput(void **state);
/**
 * Tests handling of empty value part.
 */
void crinitCfgCgroupRootNameHandlerTestEmptyInput(void **state);
#endif /* __UTEST_CGROUP_CRINIT_CFG_CGROUP_ROOT_NAME_HANDLER_H__ */
