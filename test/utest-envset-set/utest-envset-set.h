// SPDX-License-Identifier: MIT
/**
 * @file utest-envset-set.h
 * @brief Header declaring the unit tests for crinitEnvSetSet().
 */
#ifndef __UTEST_ENVSET_SET_H__
#define __UTEST_ENVSET_SET_H__

/**
 * Tests successful retrieval of an environment variable.
 */
void crinitEnvSetSetTestSuccess(void **state);
/**
 * Tests detection of NULL pointer input.
 */
void crinitEnvSetSetTestNullInput(void **state);

#endif /* __UTEST_ENVSET_SET_H__ */
