/**
 * @file utest-envset-dup.h
 * @brief Header declaring the unit tests for EBCL_envSetDu[().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __UTEST_ENVSET_DUP_H__
#define __UTEST_ENVSET_DUP_H__

/**
 * Tests successful duplication of an environment set.
 */
void EBCL_envSetDupTestSuccess(void **state);
/**
 * Tests detection of NULL pointer input.
 */
void EBCL_envSetDupTestNullInput(void **state);

#endif /* __UTEST_ENVSET_DUP_H__ */
