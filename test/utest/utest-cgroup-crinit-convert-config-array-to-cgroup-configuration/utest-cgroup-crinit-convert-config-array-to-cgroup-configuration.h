// SPDX-License-Identifier: MIT
/**
 * @file utest-cgroup-crinit-convert-config-array-to-cgroup-configuration.h
 * @brief Header declaring the unit tests for crinitConvertConfigArrayToCGroupConfiguration().
 */
#ifndef __UTEST_CGROUP_CRINIT_CONVERT_CONFIG_ARRAY_TO_CGROUP_CONFIGURATION_H__
#define __UTEST_CGROUP_CRINIT_CONVERT_CONFIG_ARRAY_TO_CGROUP_CONFIGURATION_H__

/**
 * Tests successful parsing of a single key value pair (e.g. "key=value")
 */
void crinitCfgCGroupParamsHandlerTestSingleKeyValueSuccess(void **state);
/**
 * Tests successful parsing of two key value pairs
 */
void crinitCfgCGroupParamsHandlerTestTwoKeyValueSuccess(void **state);
/**
 * Tests detection of NULL pointer input.
 */
void crinitCfgCGroupParamsHandlerTestNullInput(void **state);
/**
 * Tests handling of empty value part.
 */
void crinitCfgCGroupParamsHandlerTestEmptyInput(void **state);
/**
 * Tests handling of invalid input (missing '=' in the key value pair)
 */
void crinitCfgCGroupParamsHandlerTestInvalidInputMissingDelimiter(void **state);
/**
 * Tests handling of invalid input (missing cgroup name that is required if cgroup parameters are specified)
 */
void crinitCfgCGroupParamsHandlerTestInvalidInputMissingCGroupName(void **state);
#endif /* __UTEST_CGROUP_CRINIT_CONVERT_CONFIG_ARRAY_TO_CGROUP_CONFIGURATION_H__ */
