// SPDX-License-Identifier: MIT
/**
 * @file utest-envset-init.h
 * @brief Header declaring the unit tests for crinitEnvSetInit().
 */
#ifndef __UTEST_ENVSET_INIT_H__
#define __UTEST_ENVSET_INIT_H__

/**
 * Tests successful initialization of an environment set.
 */
void crinitEnvSetInitTestSuccess(void **state);
/**
 * Tests detection of NULL pointer input.
 */
void crinitEnvSetInitTestNullInput(void **state);
/**
 * Tests handling of a memory allocation error.
 */
void crinitEnvSetInitTestMallocError(void **state);

#endif /* __UTEST_ENVSET_INIT_H__ */
