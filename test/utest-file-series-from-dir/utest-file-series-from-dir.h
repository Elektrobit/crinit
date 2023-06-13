/**
 * @file utest-file-series-from-dir.h
 * @brief Header declaring the unit tests for crinitFileSeriesFromDir().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __UTEST_FILE_SERIES_FROM_DIR_H__
#define __UTEST_FILE_SERIES_FROM_DIR_H__

/**
 * Unit test for crinitFileSeriesFromDir(), successful execution.
 */
void crinitFileSeriesFromDirTestSuccess(void **state);

/**
 * Unit test for crinitFileSeriesFromDir(), parameter error.
 */
void crinitFileSeriesFromDirParamNullError(void **state);

/**
 * Unit test for crinitFileSeriesFromDir(), opendir error.
 */
void crinitFileSeriesFromDirOpendirError(void **state);

/**
 * Unit test for crinitFileSeriesFromDir(), dirfd error.
 */
void crinitFileSeriesFromDirDirfdError(void **state);

/**
 * Unit test for crinitFileSeriesFromDir(), scandir error.
 */
void crinitFileSeriesFromDirScandirError(void **state);

/**
 * Unit test for crinitFileSeriesFromDir(), init error.
 */
void crinitFileSeriesFromDirInitError(void **state);

/**
 * Unit test for crinitFileSeriesFromDir(), malloc error.
 */
void crinitFileSeriesFromDirNoMemError(void **state);

#endif /* __UTEST_FILE_SERIES_FROM_DIR_H__ */
