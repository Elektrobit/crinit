/**
 * @file utest-crinit-set-verbose.h
 * @brief Header declaring a the unit tests for EBCL_crinitSetVerbose().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __UTEST_CRINIT_SET_VERBOSE_H__
#define __UTEST_CRINIT_SET_VERBOSE_H__

/**
 * Unit test for EBCL_crinitSetVerbose(), unsuccessful case (failure of EBCL_globOptSet()).
 *
 * Checks that the right values are delivered to EBCL_globOptSet() and that -1 is returned.
 */
void EBCL_crinitSetVerboseTestGlobOptError(void **state);
/**
 * Unit test for EBCL_crinitSetVerbose(), successful case.
 *
 * Checks that the right values are delivered to EBCL_globOptSet() and that 0 is returned.
 */
void EBCL_crinitSetVerboseTestSuccess(void **state);

#endif /* __UTEST_CRINIT_SET_VERBOSE_H__ */
