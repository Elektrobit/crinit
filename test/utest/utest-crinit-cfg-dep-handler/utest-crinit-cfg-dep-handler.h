// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-cfg-elos-poll-interval-handler.h
 * @brief Header declaring the unit tests for crinitCfgElosPollIntervalHandler().
 */
#ifndef __UTEST_CFG_DEP_HANDLER_H__
#define __UTEST_CFG_DEP_HANDLER_H__

/**
 * Tests successful parsing of an existing dependency list.
 */
void crinitCfgDepHandlerTestSuccess(void **state);
/**
 * Tests fail parsing because of wrong config type.
 */
void crinitCfgDepHandlerTestErrConfigType(void **state);
/**
 * Tests unsuccessful parsing of an invalid input value.
 */
void crinitCfgDepHandlerTestInvalidValue(void **state);
/**
 * Tests detection of NULL pointer input.
 */
void crinitCfgDepHandlerTestNullInput(void **state);
/**
 * Tests handling of empty value part.
 */
void crinitCfgDepHandlerTestEmptyInput(void **state);
#endif /* __UTEST_CFG_DEP_HANDLER_H__ */
