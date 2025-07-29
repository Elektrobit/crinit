// SPDX-License-Identifier: MIT
/**
 * @file utest-cgroup-crinit-global-name-handler.h
 * @brief Header declaring the unit tests for crinitCfgCgroupGlobalNameHandler(().
 */
#ifndef __UTEST_CGROUP_CRINIT_CFG_CGROUP_GLOBAL_NAME_HANDLER_H__
#define __UTEST_CGROUP_CRINIT_CFG_CGROUP_GLOBAL_NAME_HANDLER_H__

/**
 * Tests successful parsing of an alphabetical group name (e.g. "sshd.cg") instead of an ID.
 */
void crinitCfgCgroupGlobalNameHandlerTestAlphaInputOneValueSuccess(void **state);
/**
 * Tests successful parsing of two alphabetical group names (e.g. "sshd.cg" and "http.cg") instead of an ID.
 */
void crinitCfgCgroupGlobalNameHandlerTestAlphaInputTwoValuesSuccess(void **state);
/**
 * Tests detection of NULL pointer input.
 */
void crinitCfgCgroupGlobalNameHandlerTestNullInput(void **state);
/**
 * Tests handling of empty value part.
 */
void crinitCfgCgroupGlobalNameHandlerTestEmptyInput(void **state);
#endif /* __UTEST_CGROUP_CRINIT_CFG_CGROUP_GLOBAL_NAME_HANDLER_H__ */
