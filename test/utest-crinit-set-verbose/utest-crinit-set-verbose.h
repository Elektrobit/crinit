// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-set-verbose.h
 * @brief Header declaring the unit tests for crinitClientSetVerbose().
 */
#ifndef __UTEST_CRINIT_SET_VERBOSE_H__
#define __UTEST_CRINIT_SET_VERBOSE_H__

/**
 * Unit test for crinitClientSetVerbose(), unsuccessful case (failure of crinitGlobOptSet()).
 *
 * Checks that the right values are delivered to crinitGlobOptSet() and that -1 is returned.
 */
void crinitClientSetVerboseTestGlobOptError(void **state);
/**
 * Unit test for crinitClientSetVerbose(), successful case.
 *
 * Checks that the right values are delivered to crinitGlobOptSet() and that 0 is returned.
 */
void crinitClientSetVerboseTestSuccess(void **state);

#endif /* __UTEST_CRINIT_SET_VERBOSE_H__ */
