/**
 * @file utest-init-file-series.h
 * @brief Header declaring the unit tests for crinitSuffixFilter().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
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
