// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-cfg-user-handler.h
 * @brief Header declaring the unit tests for crinitCfgUserHandler().
 */
#ifndef __UTEST_CFG_USER_HANDLER_H__
#define __UTEST_CFG_USER_HANDLER_H__

/**
 * Tests successful parsing of a numeric user ID.
 */
void crinitCfgUserHandlerTestNumericSuccess(void **state);
/**
 * Tests successful parsing of a alphabetical user name (e.g. "www-run") instead of an ID.
 */
void crinitCfgUserHandlerTestAlphaInputSuccess(void **state);
/**
 * Tests unsuccessful parsing of a negative numeric user ID.
 */
void crinitCfgUserHandlerTestNegativeInput(void **state);
/**
 * Tests detection of NULL pointer input.
 */
void crinitCfgUserHandlerTestNullInput(void **state);
/**
 * Tests handling of empty value part.
 */
void crinitCfgUserHandlerTestEmptyInput(void **state);
#endif /* __UTEST_CFG_USER_HANDLER_H__ */
