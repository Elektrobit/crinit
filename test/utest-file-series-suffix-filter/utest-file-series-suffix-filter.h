/**
 * @file utest-init-file-series.h
 * @brief Header declaring the unit tests for EBCL_suffixFilter().
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

bool EBCL_suffixFilter(const char *name, const char *suffix);

/**
 * Unit test for EBCL_suffixFilter(), successful execution.
 */
void EBCL_suffixFilterTestSuccess(void **state);

#endif /* __UTEST_SUFFIX_FILTER_H__ */
