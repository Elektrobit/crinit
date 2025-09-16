// SPDX-License-Identifier: MIT
/**
 * @file utest-cgroup-crinit-root-params-handler.h
 * @brief Header declaring the unit tests for crinitCfgCgroupRootParamsHandler(().
 */
#ifdef ENABLE_CGROUP
#ifndef __UTEST_CGROUP_CRINIT_CFG_CGROUP_ROOT_PARAMS_HANDLER_H__
#define __UTEST_CGROUP_CRINIT_CFG_CGROUP_ROOT_PARAMS_HANDLER_H__

/**
 * Tests successful parsing of a single key value pair (e.g. "key=value")
 */
void crinitCfgCgroupRootParamsHandlerTestSingleKeyValueSuccess(void **state);
/**
 * Tests detection of NULL pointer input.
 */
void crinitCfgCgroupRootParamsHandlerTestNullInput(void **state);
/**
 * Tests handling of empty value part.
 */
void crinitCfgCgroupRootParamsHandlerTestEmptyInput(void **state);
#endif /* __UTEST_CGROUP_CRINIT_CFG_CGROUP_ROOT_PARAMS_HANDLER_H__ */
#endif
