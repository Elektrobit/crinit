/**
 * @file fseries.h
 * @brief Header defining functions related to the handling of a series of filenames within a directory.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __FSERIES_H__
#define __FSERIES_H__

#ifdef CRINIT_FSERIES_TESTING
#define TESTABLE __attribute__((weak))
#else
#define TESTABLE
#endif

#include <stdbool.h>
#include <stddef.h>

/**
 * Data type holding a series of files inside a specific directory.
 */
typedef struct ebcl_FileSeries_t {
    char **fnames;  ///< Dynamic array of string pointers backed by a single continuous dynamically allocated string.
                    ///< Outer array is terminated by a NULL-pointer.
    size_t size;    ///< Number of valid (allocated and non-NULL) pointers in fnames.
    char *baseDir;  ///< The dirname of the filenames in fnames.
} ebcl_FileSeries_t;

/**
 * Generates an ebcl_FileSeries_t instance by scanning a given directory for regular files.
 *
 * Uses scandir() with filters.
 *
 * @param fse          Return pointer for the resulting file series, will contain allocated memory that can be freed
 *                     via EBCL_destroyFileSeries().
 * @param path         Path to the directory to scan.
 * @param fileSuffix   File extension to filter results by.
 * @param followLinks  If symbolic links to regular files matching \a fileSuffix should be included or not.
 *
 * @return  0 on success, -1 otherwise.
 */
int EBCL_fileSeriesFromDir(ebcl_FileSeries_t *fse, const char *path, const char *fileSuffix, bool followLinks);
/** Creates an ebcl_FileSeries_t instance by emplacing a pre-created array of strings.
 *
 *  Under the assumption, \a strArr is allocated as an outer array of pointers into a single dynamically allocated
 *  backing string beginning at the first pointer (as EBCL_fileSeriesFromDir() does it), EBCL_destroyFileSeries() can
 *  be used for deallocation. If that is not the case, ebcl_FileSeries_t::fnames needs to be manually freed as
 *  necessary and set to NULL before it is safe to call EBCL_destroyFileSeries().
 *
 *  @param fse      Return pointer for the resulting file series, memory for ebcl_FileSeries_t::baseDir will be
 *                  allocated.
 *  @param baseDir  Base directory of the files in the series.
 *  @param strArr   Array of strings to be emplaced as ebcl_FileSeries_t::fnames.
 *
 *  @return  0 on success, -1 otherwise.
 */
int EBCL_fileSeriesFromStrArr(ebcl_FileSeries_t *fse, const char *baseDir, char **strArr);
/**
 * Frees memory associated with an ebcl_FileSeries_t.
 *
 * @param fse  The ebcl_FileSeries_t whose memory shall be deallocated.
 */
void EBCL_destroyFileSeries(ebcl_FileSeries_t *fse);
/**
 * Initialize an ebcl_FileSeries_t with a given number of empty pointers.
 *
 * Sets initial state and then uses EBCL_resizeFileSeries() internally to allocate space for the pointers.
 *
 * @param fse          The ebcl_FileSeries_t to initialize.
 * @param numElements  The number of pointers in ebcl_FileSeries_t::fnames to allocate. No memory for the backing string
 *                     is allocated at this point.
 * @param baseDir      Base directory of the new file series to be set.
 *
 *  @return  0 on success, -1 otherwise.
 */
TESTABLE int EBCL_initFileSeries(ebcl_FileSeries_t *fse, size_t numElements, const char *baseDir);
/**
 * Grow or shrink the number of string pointers in a file series.
 *
 * May be used on an uninitialized ebcl_FileSeries_t if fnames is set to NULL before the call. Memory of the backing
 * string is unaffected.
 *
 * @param fse          The ebcl_FileSeries_t to modify.
 * @param numElements  The new number of pointers that fse shall have. Can be lower, higher or the same as the current
 *                     state.
 *
 * @return  0 on success, -1 otherwise.
 */
int EBCL_resizeFileSeries(ebcl_FileSeries_t *fse, size_t numElements);

#endif /* __FSERIES_H__ */
