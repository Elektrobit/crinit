/**
 * @file utest-init-file-series.h
 * @brief Header declaring the unit tests for EBCL_statFilter().
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

bool EBCL_statFilter(const char *name, int baseDirFd, bool followLinks);

/**
 * Unit test for EBCL_statFilter(), successful execution.
 */
void EBCL_statFilterTestSuccess(void **state);

/**
 * Unit test for EBCL_statFilter(), fstatat fail.
 */
void EBCL_statFilterTestFstatatFail(void **state);

/**
 * Unit test for EBCL_statFilter(), S_ISREG fail.
 */
void EBCL_statFilterTestSisregFail(void **state);

#endif /* __UTEST_STAT_FILTER_H__ */