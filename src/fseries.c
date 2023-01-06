/**
 * @file fseries.c
 * @brief Implementation file related to the handling of a series of filenamea.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#define _GNU_SOURCE  // Needed for O_PATH.

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

typedef struct ebcl_DirScanState_t {
    const char *fileSuffix;
    bool followLinks;
    int baseDirFd;
    pthread_mutex_t dirScanLock;
} ebcl_DirScanState_t;

static ebcl_DirScanState_t EBCL_scState = {NULL, false, -1, PTHREAD_MUTEX_INITIALIZER};

static int EBCL_scanDirFilter(const struct dirent *dent);
static inline bool EBCL_suffixFilter(const char *name, const char *suffix);
static inline bool EBCL_statFilter(const char *name, int baseDirFd, bool followLinks);

void EBCL_destroyFileSeries(ebcl_FileSeries_t *fse) {
    if (fse == NULL || fse->fnames == NULL) {
        return;
    }
    for (size_t i = 0; i <= fse->size; i++) {
        free(fse->fnames[i]);
        fse->fnames[i] = NULL;
    }
    free(fse->fnames);
    fse->fnames = NULL;
    free(fse->baseDir);
    fse->baseDir = NULL;
}

int EBCL_initFileSeries(ebcl_FileSeries_t *fse, size_t numElements, const char *baseDir) {
    if (fse == NULL) {
        EBCL_errPrint("File series struct to initialize must not be NULL.");
        return -1;
    }
    fse->fnames = NULL;
    fse->size = 0;
    fse->baseDir = NULL;
    if (baseDir != NULL) {
        fse->baseDir = strdup(baseDir);
        if (fse->baseDir == NULL) {
            EBCL_errnoPrint("Could not duplicate base directory string in file series.");
            return -1;
        }
    }
    return EBCL_resizeFileSeries(fse, numElements);
}

int EBCL_resizeFileSeries(ebcl_FileSeries_t *fse, size_t numElements) {
    if (fse == NULL) {
        EBCL_errPrint("File series struct to resize must not be NULL.");
        return -1;
    }
    if (numElements == fse->size) {
        return 0;
    }
    char **newPtr = realloc(fse->fnames, numElements + 1);
    if (newPtr == NULL) {
        EBCL_errnoPrint("Could not reallocate filename array of file series to size %zu.", numElements);
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

int EBCL_fileSeriesFromDir(ebcl_FileSeries_t *fse, const char *path, const char *fileSuffix, bool followLinks) {
    if (fse == NULL) {
        EBCL_errPrint("Return pointer must not be NULL.");
        return -1;
    }
    if (path == NULL) {
        EBCL_errPrint("Directory to scan must not be NULL.");
        return -1;
    }

    DIR *scd = opendir(path);
    if (scd == NULL) {
        EBCL_errnoPrint("Could not open directory at '%s' for scanning.", path);
        return -1;
    }
    int scdfd = dirfd(scd);
    if (scdfd == -1) {
        EBCL_errnoPrint("Could not get file descriptor from directory stream.");
        return -1;
    }

    struct dirent **scanList = NULL;
    int scanRes = -1;

    errno = pthread_mutex_lock(&EBCL_scState.dirScanLock);
    if (errno != 0) {
        EBCL_errnoPrint("Could not queue up for mutex lock.");
        closedir(scd);
        return -1;
    }

    EBCL_scState.baseDirFd = scdfd;
    EBCL_scState.fileSuffix = fileSuffix;
    EBCL_scState.followLinks = followLinks;

    scanRes = scandir(path, &scanList, EBCL_scanDirFilter, alphasort);

    EBCL_scState.baseDirFd = -1;
    EBCL_scState.fileSuffix = NULL;
    EBCL_scState.followLinks = false;

    pthread_mutex_unlock(&EBCL_scState.dirScanLock);

    closedir(scd);

    if (scanRes == -1) {
        EBCL_errnoPrint("Could not scan directory '%s'", path);
        return -1;
    }

    if (EBCL_initFileSeries(fse, scanRes, path) == -1) {
        EBCL_errPrint("Could not initialize file series struct holding %d elements.", scanRes);
        return -1;
    }

    for (size_t i = 0; i < fse->size; i++) {
        fse->fnames[i] = strdup(scanList[i]->d_name);
        if (fse->fnames[i] == NULL) {
            EBCL_errnoPrint("Could not duplicate string from directory scan.");
            fse->size = i + 1;
            EBCL_destroyFileSeries(fse);
            return -1;
        }
    }

    return 0;
}

static inline bool EBCL_suffixFilter(const char *name, const char *suffix) {
    if (name == NULL) {
        return false;
    }
    if (suffix == NULL) {
        return true;
    }
    if (strlen(suffix) > strlen(name)) {
        return false;
    }
    const char *cmpStart = name + ((off_t)strlen(name) - (off_t)strlen(suffix));
    return (strcmp(cmpStart, suffix) == 0);
}

static inline bool EBCL_statFilter(const char *name, int baseDirFd, bool followLinks) {
    if (name == NULL) {
        return false;
    }

    int fstFlags = followLinks ? AT_SYMLINK_NOFOLLOW : 0;
    struct stat stbuf;

    if (fstatat(baseDirFd, name, &stbuf, fstFlags) == -1) {
        EBCL_errnoPrint("Could not stat '%s'.", name);
        return false;
    }

    if (S_ISREG(stbuf.st_mode)) {
        return true;
    }
    return false;
}

static int EBCL_scanDirFilter(const struct dirent *dent) {
    if (dent == NULL) {
        return 0;
    }
    if (EBCL_scState.fileSuffix != NULL) {
        if (EBCL_suffixFilter(dent->d_name, EBCL_scState.fileSuffix) == 0) {
            return 0;
        }
    }
    if (EBCL_statFilter(dent->d_name, EBCL_scState.baseDirFd, EBCL_scState.followLinks)) {
        return 1;
    }
    return 0;
}
