// SPDX-License-Identifier: MIT
/**
 * @file utest-file-series-from-str-arr.h
 * @brief Header declaring the unit tests for crinitFileSeriesFromStrArr().
 */
#ifndef __UTEST_FILE_SERIES_FROM_STR_ARR_H__
#define __UTEST_FILE_SERIES_FROM_STR_ARR_H__

/**
 * Unit test for crinitFileSeriesFromStrArr(), successful execution.
 */
void crinitFileSeriesFromStrArrTestSuccess(void **state);

/**
 * Unit test for crinitFileSeriesFromStrArr(), when strdup returns NULL.
 */
void crinitFileSeriesFromStrArrTestNoMemError(void **state);

/**
 * Unit test for crinitFileSeriesFromStrArr(), given null parameter.
 */
void crinitFileSeriesFromStrArrTestNullParamError(void **state);

#endif /* __UTEST_FILE_SERIES_FROM_STR_ARR_H__ */
