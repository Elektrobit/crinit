/**
 * @file utest-file-series-from-dir.h
 * @brief Header declaring the unit tests for EBCL_fileSeriesFromDir().
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
 * Unit test for EBCL_fileSeriesFromDir(), successful execution.
 */
void EBCL_fileSeriesFromDirTestSuccess(void **state);

/**
 * Unit test for EBCL_fileSeriesFromDir(), parameter error.
 */
void EBCL_fileSeriesFromDirParamNullError(void **state);

/**
 * Unit test for EBCL_fileSeriesFromDir(), opendir error.
 */
void EBCL_fileSeriesFromDirOpendirError(void **state);

/**
 * Unit test for EBCL_fileSeriesFromDir(), dirfd error.
 */
void EBCL_fileSeriesFromDirDirfdError(void **state);

/**
 * Unit test for EBCL_fileSeriesFromDir(), scandir error.
 */
void EBCL_fileSeriesFromDirScandirError(void **state);

/**
 * Unit test for EBCL_fileSeriesFromDir(), init error.
 */
void EBCL_fileSeriesFromDirInitError(void **state);

/**
 * Unit test for EBCL_fileSeriesFromDir(), malloc error.
 */
void EBCL_fileSeriesFromDirNoMemError(void **state);

#endif /* __UTEST_FILE_SERIES_FROM_DIR_H__ */
