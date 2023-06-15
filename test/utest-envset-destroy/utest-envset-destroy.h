/**
 * @file utest-envset-destroy.h
 * @brief Header declaring the unit tests for crinitEnvSetDestroy().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __UTEST_ENVSET_DESTROY_H__
#define __UTEST_ENVSET_DESTROY_H__

/**
 * Tests successful desruction of an environment set.
 *
 * Will check that all allocated pointers in the EnvSet are freed.
 */
void crinitEnvSetDestroyTestSuccess(void **state);
/**
 * Tests detection of NULL pointer input.
 */
void crinitEnvSetDestroyTestNullInput(void **state);

#endif /* __UTEST_ENVSET_DESTROY_H__ */
