/**
 * @file utest-envset-set.h
 * @brief Header declaring the unit tests for EBCL_envSetSet().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __UTEST_ENVSET_SET_H__
#define __UTEST_ENVSET_SET_H__

/**
 * Tests successful retrieval of an environment variable.
 */
void EBCL_envSetSetTestSuccess(void **state);
/**
 * Tests detection of NULL pointer input.
 */
void EBCL_envSetSetTestNullInput(void **state);

#endif /* __UTEST_ENVSET_SET_H__ */
