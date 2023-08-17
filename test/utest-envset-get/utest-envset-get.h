// SPDX-License-Identifier: MIT
/**
 * @file utest-envset-get.h
 * @brief Header declaring the unit tests for crinitEnvSetGet().
 */
#ifndef __UTEST_ENVSET_GET_H__
#define __UTEST_ENVSET_GET_H__

/**
 * Tests successful retrieval of an environment variable.
 */
void crinitEnvSetGetTestSuccess(void **state);
/**
 * Tests detection of NULL pointer input.
 */
void crinitEnvSetGetTestNullInput(void **state);
/**
 * Tests unsuccessful retrieval of non-existent environment variable.
 */
void crinitEnvSetGetTestNotFound(void **state);

#endif /* __UTEST_ENVSET_GET_H__ */
