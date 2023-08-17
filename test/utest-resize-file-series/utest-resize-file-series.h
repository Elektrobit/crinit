// SPDX-License-Identifier: MIT
/**
 * @file utest-resize-file-series.h
 * @brief Header declaring the unit tests for crinitResizeFileSeries().
 */
#ifndef __UTEST_RESIZE_FILE_SERIES_H__
#define __UTEST_RESIZE_FILE_SERIES_H__

/**
 * Unit test for crinitResizeFileSeries(), successful execution.
 */
void crinitResizeFileSeriesTestSuccess(void **state);

/**
 * Unit test for crinitResizeFileSeries(), fails to reallocate memory.
 */
void crinitResizeFileSeriesTestNoMemError(void **state);

/**
 * Unit test for crinitResizeFileSeries(), given file series is NULL.
 */
void crinitResizeFileSeriesTestFseNullError(void **state);

/**
 * Unit test for crinitResizeFileSeries(), shrinking to zero fails.
 */
void crinitResizeFileSeriesTestShrinkZeroError(void **state);

#endif /* __UTEST_RESIZE_FILE_SERIES_H__ */
