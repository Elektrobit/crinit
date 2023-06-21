/**
 * @file fseries.c
 * @brief Implementation file related to the handling of a series of filenames within a directory.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "fseries.h"

#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "confparse.h"
#include "logio.h"

#ifdef CRINIT_FSERIES_TESTING
#define TESTABLE_STATIC __attribute__((weak))
#else
#define TESTABLE_STATIC static
#endif

/**
 * Global state/option variable to store options and state of a directory scan for the dirscan() filters.
 *
 * Used to configure the filters for scandir() in crinitFileSeriesFromDir(). Needs to be used as a compilation-unit-
 * global variable as scandir() does not allow arbitrary arguments to the filters.
 */
static struct {
    const char *fileSuffix;       ///< The extension of the files we want to scan for.
    bool followLinks;             ///< If we should follow symlinks that have the correct
                                  ///< fileSuffix. If `false`, symlinks will be filtered out of the list.
    int baseDirFd;                ///< File descriptor of the opened base directory for the search.
    pthread_mutex_t dirScanLock;  ///< Mutex to synchronize concurrent accesses to this data, must be held
                                  ///< during scandir().
} crinitScState = {.baseDirFd = -1, .dirScanLock = PTHREAD_MUTEX_INITIALIZER};

/**
 * Result filter to be given to scandir() via function pointer.
 *
 * Uses crinitScState to setup and run crinitSuffixFilter() and crinitStatFilter().
 *
 * See also [scandir(3) man page](https://man7.org/linux/man-pages/man3/scandir.3.html).
 *
 * @param dent  Pointer to a directory entry.
 *
 * @return  1 if \a dent should be included in the final result list, 0 if not.
 */
TESTABLE_STATIC int criitScanDirFilter(const struct dirent *dent);
/**
 * Filters strings by suffix.
 *
 * @param name    String to be fed to the filter.
 * @param suffix  Suffix to check for.
 *
 * @return  true if \a name ends with \a suffix, false if not.
 */
TESTABLE_STATIC bool crinitSuffixFilter(const char *name, const char *suffix);
/**
 * Filters out everything that is not a regular file (or a symlink to it, depending on settings).
 *
 * @param name  Filename of the file to be fed to the filter.
 * @param baseDirFd  Opened file descriptor of the directory, the file in question resides.
 * @param followLinks  If symbolic links should be followed to its destination before filtering (true) or generally
 *                     filtered out (false).
 *
 * @return  true if \a name refers to a regular file, false if not.
 */
TESTABLE_STATIC int crinitStatFilter(const char *name, int baseDirFd, bool followLinks);
/**
 * Free return pointer(s) from scandir().
 *
 * Will free everything, scandir() has allocated according to the
 * [scandir(3) man page](https://man7.org/linux/man-pages/man3/scandir.3.html).
 */
TESTABLE_STATIC void crinitFreeScandirList(struct dirent **scanList, int size);

TESTABLE void crinitDestroyFileSeries(crinitFileSeries_t *fse) {
    if (fse == NULL) {
        return;
    }
    if (fse->fnames != NULL) {
        free(fse->fnames[0]);
        fse->fnames[0] = NULL;
        free(fse->fnames);
        fse->fnames = NULL;
    }
    free(fse->baseDir);
    fse->baseDir = NULL;
    fse->size = 0;
}

TESTABLE int crinitInitFileSeries(crinitFileSeries_t *fse, size_t numElements, const char *baseDir) {
    if (fse == NULL) {
        crinitErrPrint("File series struct to initialize must not be NULL.");
        return -1;
    }
    fse->fnames = NULL;
    fse->size = 0;
    fse->baseDir = NULL;
    if (baseDir != NULL) {
        fse->baseDir = strdup(baseDir);
        if (fse->baseDir == NULL) {
            crinitErrnoPrint("Could not duplicate base directory string in file series.");
            return -1;
        }
    }
    return crinitResizeFileSeries(fse, numElements);
}

int crinitResizeFileSeries(crinitFileSeries_t *fse, size_t numElements) {
    if (fse == NULL) {
        crinitErrPrint("File series struct to resize must not be NULL.");
        return -1;
    }
    if (numElements == fse->size) {
        return 0;
    }
    if (numElements == 0) {
        crinitErrPrint("File series struct shrink to 0 is not supported.");
        return -1;
    }
    char **newPtr = realloc(fse->fnames, (numElements + 1) * sizeof(char *));
    if (newPtr == NULL) {
        crinitErrnoPrint("Could not reallocate filename array of file series to size %zu.", numElements);
        return -1;
    }
    fse->fnames = newPtr;
    int elementDiff = numElements - fse->size;
    for (size_t i = fse->size; i <= fse->size + elementDiff; i++) {
        fse->fnames[i] = NULL;
    }
    fse->size = numElements;
    return 0;
}

int crinitFileSeriesFromDir(crinitFileSeries_t *fse, const char *path, const char *fileSuffix, bool followLinks) {
    if (fse == NULL) {
        crinitErrPrint("Return pointer must not be NULL.");
        return -1;
    }
    if (path == NULL) {
        crinitErrPrint("Directory to scan must not be NULL.");
        return -1;
    }

    DIR *scd = opendir(path);
    if (scd == NULL) {
        crinitErrnoPrint("Could not open directory at '%s' for scanning.", path);
        return -1;
    }
    int scdfd = dirfd(scd);
    if (scdfd == -1) {
        crinitErrnoPrint("Could not get file descriptor from directory stream.");
        closedir(scd);
        return -1;
    }

    struct dirent **scanList = NULL;
    int scanRes = -1;

    errno = pthread_mutex_lock(&crinitScState.dirScanLock);
    if (errno != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        closedir(scd);
        return -1;
    }

    crinitScState.baseDirFd = scdfd;
    crinitScState.fileSuffix = fileSuffix;
    crinitScState.followLinks = followLinks;

    scanRes = scandir(path, &scanList, criitScanDirFilter, alphasort);

    crinitScState.baseDirFd = -1;
    crinitScState.fileSuffix = NULL;
    crinitScState.followLinks = false;

    pthread_mutex_unlock(&crinitScState.dirScanLock);

    if (scanRes == -1) {
        crinitErrnoPrint("Could not scan directory '%s'", path);
        closedir(scd);
        return -1;
    }

    closedir(scd);

    if (crinitInitFileSeries(fse, scanRes, path) == -1) {
        crinitErrPrint("Could not initialize file series struct holding %d elements.", scanRes);
        return -1;
    }

    size_t backingStrAllocLen = 0;
    for (size_t i = 0; i < fse->size; i++) {
        backingStrAllocLen += strlen(scanList[i]->d_name) + 1;
    }

    fse->fnames[0] = malloc(backingStrAllocLen * sizeof(char));
    if (fse->fnames[0] == NULL) {
        crinitErrnoPrint("Could not allocate memory for file series backing string.");
        crinitDestroyFileSeries(fse);
        return -1;
    }

    char *runner = fse->fnames[0];
    for (size_t i = 0; i < fse->size; i++) {
        fse->fnames[i] = runner;
        runner = stpcpy(runner, scanList[i]->d_name) + 1;
    }
    crinitFreeScandirList(scanList, scanRes);
    return 0;
}

int crinitFileSeriesFromStrArr(crinitFileSeries_t *fse, const char *baseDir, char **strArr) {
    if (fse == NULL || baseDir == NULL || strArr == NULL) {
        crinitErrPrint("Input parameters must not be NULL.");
        return -1;
    }

    fse->fnames = strArr;

    char **ptr = strArr;
    while (*ptr != NULL) {
        ptr++;
    }
    fse->size = ptr - strArr;

    fse->baseDir = strdup(baseDir);
    if (fse->baseDir == NULL) {
        crinitErrnoPrint("Could not duplicate string for base directory of file series.");
        return -1;
    }
    return 0;
}

bool crinitSuffixFilter(const char *name, const char *suffix) {
    if (suffix == NULL || strcmp(suffix, "") == 0) {
        return true;
    }
    const char *cmpStart = name + ((off_t)strlen(name) - (off_t)strlen(suffix));
    /* ensure name is longer than suffix and suffix matches */
    return (cmpStart >= name) && (strcmp(cmpStart, suffix) == 0);
}

int crinitStatFilter(const char *name, int baseDirFd, bool followLinks) {
    int fstFlags = followLinks ? 0 : AT_SYMLINK_NOFOLLOW;
    struct stat stbuf;

    if (fstatat(baseDirFd, name, &stbuf, fstFlags) == -1) {
        crinitErrnoPrint("Could not stat '%s'.", name);
        return false;
    }

    return S_ISREG(stbuf.st_mode);
}

int criitScanDirFilter(const struct dirent *dent) {
    if (dent == NULL || dent->d_name == NULL) {
        return 0;
    }
    return crinitStatFilter(dent->d_name, crinitScState.baseDirFd, crinitScState.followLinks) &&
           crinitSuffixFilter(dent->d_name, crinitScState.fileSuffix);
}

void crinitFreeScandirList(struct dirent **scanList, int size) {
    if (scanList == NULL) {
        return;
    }
    while (size--) {
        free(scanList[size]);
    }
    free(scanList);
}
