/**
 * @file utest-init-file-series.h
 * @brief Header declaring the unit tests for EBCL_initFileSeries().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __UTEST_INIT_FILE_SERIES_H__
#define __UTEST_INIT_FILE_SERIES_H__

/**
 * Unit test for EBCL_initFileSeries(), successful execution.
 */
void EBCL_initFileSeriesTestSuccess(void **state);

/**
 * Unit test for EBCL_initFileSeries(), copying of base dir fails.
 */
void EBCL_initFileSeriesTestNoMemError(void **state);

/**
 * Unit test for EBCL_initFileSeries(), given file series is NULL.
 */
void EBCL_initFileSeriesTestFseNullError(void **state);

#endif /* __UTEST_INIT_FILE_SERIES_H__ */
