// SPDX-License-Identifier: MIT
/**
 * @file utest-init-file-series.h
 * @brief Header declaring the unit tests for crinitInitFileSeries().
 */
#ifndef __UTEST_INIT_FILE_SERIES_H__
#define __UTEST_INIT_FILE_SERIES_H__

/**
 * Unit test for crinitInitFileSeries(), successful execution.
 */
void crinitInitFileSeriesTestSuccess(void **state);

/**
 * Unit test for crinitInitFileSeries(), copying of base dir fails.
 */
void crinitInitFileSeriesTestNoMemError(void **state);

/**
 * Unit test for crinitInitFileSeries(), given file series is NULL.
 */
void crinitInitFileSeriesTestFseNullError(void **state);

#endif /* __UTEST_INIT_FILE_SERIES_H__ */
