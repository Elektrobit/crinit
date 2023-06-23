/**
 * @file utest-inih-configuration.h
 * @brief Header declaring the regression test for the libinih compile-time-configuration.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
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
