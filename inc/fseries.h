// SPDX-License-Identifier: MIT
/**
 * @file fseries.h
 * @brief Header defining functions related to the handling of a series of filenames within a directory.
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
typedef struct crinitFileSeries_t {
    char **fnames;  ///< Dynamic array of string pointers backed by a single continuous dynamically allocated string.
                    ///< Outer array is terminated by a NULL-pointer.
    size_t size;    ///< Number of valid (allocated and non-NULL) pointers in fnames.
    char *baseDir;  ///< The dirname of the filenames in fnames.
} crinitFileSeries_t;

/**
 * Generates an crinitFileSeries_t instance by scanning a given directory for regular files.
 *
 * Uses scandir() with filters.
 *
 * @param fse          Return pointer for the resulting file series, will contain allocated memory that can be freed
 *                     via crinitDestroyFileSeries().
 * @param path         Path to the directory to scan.
 * @param fileSuffix   File extension to filter results by.
 * @param followLinks  If symbolic links to regular files matching \a fileSuffix should be included or not.
 *
 * @return  0 on success, -1 otherwise.
 */
int crinitFileSeriesFromDir(crinitFileSeries_t *fse, const char *path, const char *fileSuffix, bool followLinks);
/** Creates an crinitFileSeries_t instance by emplacing a pre-created array of strings.
 *
 *  Under the assumption, \a strArr is allocated as an outer array of pointers into a single dynamically allocated
 *  backing string beginning at the first pointer (as crinitFileSeriesFromDir() does it), crinitDestroyFileSeries() can
 *  be used for deallocation. If that is not the case, crinitFileSeries_t::fnames needs to be manually freed as
 *  necessary and set to NULL before it is safe to call crinitDestroyFileSeries().
 *
 *  @param fse      Return pointer for the resulting file series, memory for crinitFileSeries_t::baseDir will be
 *                  allocated.
 *  @param baseDir  Base directory of the files in the series.
 *  @param strArr   Array of strings to be emplaced as crinitFileSeries_t::fnames.
 *
 *  @return  0 on success, -1 otherwise.
 */
int crinitFileSeriesFromStrArr(crinitFileSeries_t *fse, const char *baseDir, char **strArr);
/**
 * Frees memory associated with an crinitFileSeries_t.
 *
 * @param fse  The crinitFileSeries_t whose memory shall be deallocated.
 */
void crinitDestroyFileSeries(crinitFileSeries_t *fse);
/**
 * Initialize an crinitFileSeries_t with a given number of empty pointers.
 *
 * Sets initial state and then uses crinitResizeFileSeries() internally to allocate space for the pointers.
 *
 * @param fse          The crinitFileSeries_t to initialize.
 * @param numElements  The number of pointers in crinitFileSeries_t::fnames to allocate. No memory for the backing string
 *                     is allocated at this point.
 * @param baseDir      Base directory of the new file series to be set.
 *
 *  @return  0 on success, -1 otherwise.
 */
TESTABLE int crinitInitFileSeries(crinitFileSeries_t *fse, size_t numElements, const char *baseDir);
/**
 * Grow or shrink the number of string pointers in a file series.
 *
 * May be used on an uninitialized crinitFileSeries_t if fnames is set to NULL before the call. Memory of the backing
 * string is unaffected.
 *
 * @param fse          The crinitFileSeries_t to modify.
 * @param numElements  The new number of pointers that fse shall have. Can be lower, higher or the same as the current
 *                     state.
 *
 * @return  0 on success, -1 otherwise.
 */
int crinitResizeFileSeries(crinitFileSeries_t *fse, size_t numElements);

#endif /* __FSERIES_H__ */
