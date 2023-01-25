/**
 * @file utest-init-file-series.h
 * @brief Header declaring the unit tests for EBCL_freeScandirList().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __UTEST_FILE_SERIES_FREE_SCANDIR_LIST_H__
#define __UTEST_FILE_SERIES_FREE_SCANDIR_LIST_H__

#include <dirent.h>

void EBCL_freeScandirList(struct dirent **scanList, int size);

/**
 * Unit test for EBCL_freeScandirList(), successful execution.
 */
void EBCL_freeScandirListTestSuccess(void **state);

#endif /* __UTEST_FILE_SERIES_FREE_SCANDIR_LIST_H__ */