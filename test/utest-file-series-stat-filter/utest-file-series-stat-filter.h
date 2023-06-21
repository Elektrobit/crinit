/**
 * @file utest-init-file-series.h
 * @brief Header declaring the unit tests for crinitStatFilter().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __UTEST_STAT_FILTER_H__
#define __UTEST_STAT_FILTER_H__

#include <stdbool.h>

bool crinitStatFilter(const char *name, int baseDirFd, bool followLinks);

/**
 * Unit test for crinitStatFilter(), successful execution.
 */
void crinitStatFilterTestSuccess(void **state);

/**
 * Unit test for crinitStatFilter(), fstatat fail.
 */
void crinitStatFilterTestFstatatFail(void **state);

/**
 * Unit test for crinitStatFilter(), S_ISREG fail.
 */
void crinitStatFilterTestSisregFail(void **state);

#endif /* __UTEST_STAT_FILTER_H__ */
