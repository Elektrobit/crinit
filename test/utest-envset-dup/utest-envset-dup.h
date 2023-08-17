// SPDX-License-Identifier: MIT
/**
 * @file utest-envset-dup.h
 * @brief Header declaring the unit tests for crinitEnvSetDup().
 */
#ifndef __UTEST_ENVSET_DUP_H__
#define __UTEST_ENVSET_DUP_H__

/**
 * Tests successful duplication of an environment set.
 */
void crinitEnvSetDupTestSuccess(void **state);
/**
 * Tests detection of NULL pointer input.
 */
void crinitEnvSetDupTestNullInput(void **state);

#endif /* __UTEST_ENVSET_DUP_H__ */
