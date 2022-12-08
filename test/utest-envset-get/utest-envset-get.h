/**
 * @file utest-envset-get.h
 * @brief Header declaring the unit tests for EBCL_envSetGet().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __UTEST_ENVSET_GET_H__
#define __UTEST_ENVSET_GET_H__

/**
 * Tests successful retrieval of an environment variable.
 */
void EBCL_envSetGetTestSuccess(void **state);
/**
 * Tests detection of NULL pointer input.
 */
void EBCL_envSetGetTestNullInput(void **state);
/**
 * Tests unsuccessful retrieval of non-existent environment variable.
 */
void EBCL_envSetGetTestNotFound(void **state);

#endif /* __UTEST_ENVSET_GET_H__ */
