// SPDX-License-Identifier: MIT
/**
 * @file utest-globopt-defaults.h
 * @brief Header declaring the regression test for default initialization of global options.
 */
#ifndef __UTEST_GLOBOPT_DEFAULTS_H__
#define __UTEST_GLOBOPT_DEFAULTS_H__

/**
 * Regression test for default initialization of global options.
 *
 * Pre-Initializes the global option struct using a unique bit pattern (different from all defaults) and then checks
 * if parts of the pattern remain after crinitGlobOptInitDefault() which would indicate an uninitialized value, i.e.
 * a missing default.
 *
 * Allocation functions are mocked such that there is no ambiguity between the test pattern and a calloc() or strdup()
 * return value. memset() is mocked to a no-op, so that crinitGlobOptDefault() does not zero-out the test pattern before
 * initializing elements.
 */
void crinitGlobDefRegressionTest(void **state);

#endif /* __UTEST_GLOBOPT_DEFAULTS__ */
