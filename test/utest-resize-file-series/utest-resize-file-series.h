/**
 * @file utest-resize-file-series.h
 * @brief Header declaring the unit tests for EBCL_resizeFileSeries().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __UTEST_RESIZE_FILE_SERIES_H__
#define __UTEST_RESIZE_FILE_SERIES_H__

/**
 * Unit test for EBCL_resizeFileSeries(), successful execution.
 */
void EBCL_resizeFileSeriesTestSuccess(void **state);

/**
 * Unit test for EBCL_resizeFileSeries(), fails to reallocate memory.
 */
void EBCL_resizeFileSeriesTestNoMemError(void **state);

/**
 * Unit test for EBCL_resizeFileSeries(), given file series is NULL.
 */
void EBCL_resizeFileSeriesTestFseNullError(void **state);

/**
 * Unit test for EBCL_resizeFileSeries(), shrinking to zero fails.
 */
void EBCL_resizeFileSeriesTestShrinkZeroError(void **state);

#endif /* __UTEST_RESIZE_FILE_SERIES_H__ */