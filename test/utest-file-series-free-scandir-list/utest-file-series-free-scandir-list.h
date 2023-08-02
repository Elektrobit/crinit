// SPDX-License-Identifier: MIT
/**
 * @file utest-init-file-series.h
 * @brief Header declaring the unit tests for crinitFreeScandirList().
 */
#ifndef __UTEST_FILE_SERIES_FREE_SCANDIR_LIST_H__
#define __UTEST_FILE_SERIES_FREE_SCANDIR_LIST_H__

#include <dirent.h>

void crinitFreeScandirList(struct dirent **scanList, int size);

/**
 * Unit test for crinitFreeScandirList(), successful execution.
 */
void crinitFreeScandirListTestSuccess(void **state);

#endif /* __UTEST_FILE_SERIES_FREE_SCANDIR_LIST_H__ */
