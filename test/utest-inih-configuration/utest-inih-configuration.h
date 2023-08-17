// SPDX-License-Identifier: MIT
/**
 * @file utest-inih-configuration.h
 * @brief Header declaring the regression test for the libinih compile-time-configuration.
 */
#ifndef __UTEST_INIH_CONFIGURATION_H__
#define __UTEST_INIH_CONFIGURATION_H__

/**
 * Regression test for libinih compile-time configuration.
 *
 * Checks that the custom compile-time configuration for libinih defined in deps/inih/README.md is actually used and
 * followed.
 */
void crinitInihConfigurationRegressionTest(void **state);

#endif /* __UTEST_INIH_CONFIGURATION__ */
