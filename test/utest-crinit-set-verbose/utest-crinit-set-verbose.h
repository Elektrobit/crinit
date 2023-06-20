/**
 * @file utest-crinit-set-verbose.h
 * @brief Header declaring the unit tests for crinitClientSetVerbose().
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
 * Unit test for crinitClientSetVerbose(), unsuccessful case (failure of EBCL_globOptSet()).
 *
 * Checks that the right values are delivered to EBCL_globOptSet() and that -1 is returned.
 */
void crinitClientSetVerboseTestGlobOptError(void **state);
/**
 * Unit test for crinitClientSetVerbose(), successful case.
 *
 * Checks that the right values are delivered to EBCL_globOptSet() and that 0 is returned.
 */
void crinitClientSetVerboseTestSuccess(void **state);

#endif /* __UTEST_CRINIT_SET_VERBOSE_H__ */
