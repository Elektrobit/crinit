// SPDX-License-Identifier: MIT
/**
 * @file utest-init-file-series.h
 * @brief Header declaring the unit tests for crinitSuffixFilter().
 */
#ifndef __UTEST_SUFFIX_FILTER_H__
#define __UTEST_SUFFIX_FILTER_H__

#include <stdbool.h>

bool crinitSuffixFilter(const char *name, const char *suffix);

/**
 * Unit test for crinitSuffixFilter(), successful execution.
 */
void crinitSuffixFilterTestSuccess(void **state);

#endif /* __UTEST_SUFFIX_FILTER_H__ */
