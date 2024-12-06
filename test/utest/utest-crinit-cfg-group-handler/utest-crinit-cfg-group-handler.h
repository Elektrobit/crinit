// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-cfg-group-handler.h
 * @brief Header declaring the unit tests for crinitCfgGroupHandler().
 */
#ifndef __UTEST_CFG_GROUP_HANDLER_H__
#define __UTEST_CFG_GROUP_HANDLER_H__

/**
 * Tests successful parsing of a numeric group ID.
 */
void crinitCfgGroupHandlerTestNumericSuccess(void **state);
/**
 * Tests unsuccessful parsing of a alphabetical group name (e.g. "disk") instead of an ID.
 */
void crinitCfgGroupHandlerTestAlphaInput(void **state);
/**
 * Tests unsuccessful parsing of a negative numeric group ID.
 */
void crinitCfgGroupHandlerTestNegativeInput(void **state);
/**
 * Tests detection of NULL pointer input.
 */
void crinitCfgGroupHandlerTestNullInput(void **state);
/**
 * Tests handling of empty value part.
 */
void crinitCfgGroupHandlerTestEmptyInput(void **state);
#endif /* __UTEST_CFG_GROUP_HANDLER_H__ */
