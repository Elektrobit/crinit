// SPDX-License-Identifier: MIT
/**
 * @file utest-cgroup-crinit-global-params-handler.h
 * @brief Header declaring the unit tests for crinitCfgCgroupGlobalParamsHandler(().
 */
#ifdef ENABLE_CGROUP
#ifndef __UTEST_CGROUP_CRINIT_CFG_CGROUP_GLOBAL_PARAMS_HANDLER_H__
#define __UTEST_CGROUP_CRINIT_CFG_CGROUP_GLOBAL_PARAMS_HANDLER_H__

/**
 * Tests successful parsing of a single key value pair (e.g. "key=value")
 */
void crinitCfgCgroupGlobalParamsHandlerTestSingleKeyValueSuccess(void **state);
/**
 * Tests detection of NULL pointer input.
 */
void crinitCfgCgroupGlobalParamsHandlerTestNullInput(void **state);
/**
 * Tests handling of empty value part.
 */
void crinitCfgCgroupGlobalParamsHandlerTestEmptyInput(void **state);
#endif /* __UTEST_CGROUP_CRINIT_CFG_CGROUP_GLOBAL_PARAMS_HANDLER_H__ */
#endif
