/**
 * @file utest-file-series-from-str-arr.h
 * @brief Header declaring the unit tests for EBCL_fileSeriesFromStrArr().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __UTEST_FILE_SERIES_FROM_STR_ARR_H__
#define __UTEST_FILE_SERIES_FROM_STR_ARR_H__

/**
 * Unit test for EBCL_fileSeriesFromStrArr(), successful execution.
 */
void EBCL_fileSeriesFromStrArrTestSuccess(void **state);

/**
 * Unit test for EBCL_fileSeriesFromStrArr(), when strdup returns NULL.
 */
void EBCL_fileSeriesFromStrArrTestNoMemError(void **state);

/**
 * Unit test for EBCL_fileSeriesFromStrArr(), given null parameter.
 */
void EBCL_fileSeriesFromStrArrTestNullParamError(void **state);

#endif /* __UTEST_FILE_SERIES_FROM_STR_ARR_H__ */