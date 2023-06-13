/**
 * @file utest-envset-init.h
 * @brief Header declaring the unit tests for crinitEnvSetInit().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
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
