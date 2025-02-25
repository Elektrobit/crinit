// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-cfg-elos-poll-interval-handler.h
 * @brief Header declaring the unit tests for crinitCfgElosPollIntervalHandler().
 */
#ifndef __UTEST_CFG_ELOS_POLL_INTERVAL_HANDLER_H__
#define __UTEST_CFG_ELOS_POLL_INTERVAL_HANDLER_H__

/**
 * Tests successful parsing of an existing executable.
 */
void crinitCfgElosPollIntervalHandlerTestRuntimeSettingSuccess(void **state);
/**
 * Tests default value.
 */
void crinitCfgElosPollIntervalDefaultValue(void **state);
/**
 * Tests unsuccessful parsing of an invalid input value.
 */
void crinitCfgElosPollIntervalHandlerTestInvalidInput(void **state);
/**
 * Tests detection of NULL pointer input.
 */
void crinitCfgElosPollIntervalHandlerTestNullInput(void **state);
/**
 * Tests handling of empty value part.
 */
void crinitCfgElosPollIntervalHandlerTestEmptyInput(void **state);
#endif /* __UTEST_CFG_ELOS_POLL_INTERVAL_HANDLER_H__ */
