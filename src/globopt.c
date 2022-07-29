/**
 * @file globopt.c
 * @brief Implementation of global option storage.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "globopt.h"

#include <pthread.h>
#include <stdlib.h>

#include "logio.h"

/** Array to hold pointers to the global option values, one pointer per global option. **/
static void *EBCL_globOptArr[EBCL_GLOBOPT_END - EBCL_GLOBOPT_START] = {NULL};
/** Mutex to synchronize access to globOptArr **/
static pthread_mutex_t EBCL_optLock = PTHREAD_MUTEX_INITIALIZER;

int EBCL_globOptInitDefault(void) {
    for (ebcl_GlobOptKey_t i = EBCL_GLOBOPT_START; i < EBCL_GLOBOPT_END; i++) {
        switch (i) {
            case EBCL_GLOBOPT_DEBUG: {
                bool def = EBCL_GLOBOPT_DEFAULT_DEBUG;
                if (EBCL_globOptSetBoolean(i, &def) == -1) {
                    EBCL_errPrint("Could not set default value for global option \'DEBUG\'.");
                    return -1;
                }
            } break;
            case EBCL_GLOBOPT_FILESIGS: {
                bool def = EBCL_GLOBOPT_DEFAULT_FILESIGS;
                if (EBCL_globOptSetBoolean(i, &def) == -1) {
                    EBCL_errPrint("Could not set default value for global option \'FILE_SIGS_NEEDED\'.");
                    return -1;
                }
            } break;
            case EBCL_GLOBOPT_TASKDIR:
                if (EBCL_globOptSetString(i, EBCL_GLOBOPT_DEFAULT_TASKDIR) == -1) {
                    EBCL_errPrint("Could not set default value for global option \'TASKDIR\'.");
                }
                break;
            case EBCL_GLOBOPT_SHDGRACEP: {
                unsigned long long def = EBCL_GLOBOPT_DEFAULT_SHDGRACEP;
                if (EBCL_globOptSetUnsignedLL(i, &def) == -1) {
                    EBCL_errPrint("Could not set default value for global option \'SHUTDOWN_GRACE_PERIOD_US\'.");
                }
            } break;
            case EBCL_GLOBOPT_START:
            default:
                if (EBCL_globOptSet(i, NULL, 0) == -1) {
                    EBCL_errPrint("Could not set unknown global option to default NULL pointer.");
                    return -1;
                }
            case EBCL_GLOBOPT_END:
                break;
        }
    }
    return 0;
}

int EBCL_globOptSet(ebcl_GlobOptKey_t key, const void *val, size_t sz) {
    if ((errno = pthread_mutex_lock(&EBCL_optLock)) == -1) {
        EBCL_errnoPrint("Could not wait for global option array mutex lock.");
        return -1;
    }
    size_t idx = (size_t)key;
    if (EBCL_globOptArr[idx] != NULL) {
        free(EBCL_globOptArr[idx]);
    }
    if (val == NULL || sz == 0) {
        EBCL_globOptArr[idx] = NULL;
    } else {
        EBCL_globOptArr[idx] = malloc(sz);
        if (EBCL_globOptArr[idx] == NULL) {
            EBCL_errPrint("Could not allocate memory for global option.");
            pthread_mutex_unlock(&EBCL_optLock);
            return -1;
        }
        memcpy(EBCL_globOptArr[idx], val, sz);
    }
    pthread_mutex_unlock(&EBCL_optLock);
    return 0;
}

int EBCL_globOptGet(ebcl_GlobOptKey_t key, void *val, size_t sz) {
    if (val == NULL || sz == 0) {
        EBCL_errPrint("Return value pointer must not be NULL and at least 1 Byte must be read.");
        return -1;
    }
    if ((errno = pthread_mutex_lock(&EBCL_optLock)) == -1) {
        EBCL_errnoPrint("Could not wait for global option array mutex lock.");
        return -1;
    }
    size_t idx = (size_t)key;
    if (EBCL_globOptArr[idx] == NULL) {
        EBCL_errPrint("Could not read global option as it is uninitialized.");
        pthread_mutex_unlock(&EBCL_optLock);
        return -1;
    }
    memcpy(val, EBCL_globOptArr[idx], sz);
    pthread_mutex_unlock(&EBCL_optLock);
    return 0;
}

int EBCL_globOptSetString(ebcl_GlobOptKey_t key, const char *str) {
    if (str == NULL) {
        EBCL_errPrint("Input string must not be NULL.");
        return -1;
    }
    size_t len = strlen(str) + 1;
    char *copyData = malloc(len + sizeof(size_t));
    if (copyData == NULL) {
        EBCL_errnoPrint("Could not allocate memory for global option string.");
        return -1;
    }
    memcpy(copyData, &len, sizeof(size_t));
    memcpy(copyData + sizeof(size_t), str, len);
    if (EBCL_globOptSet(key, copyData, len + sizeof(size_t)) == -1) {
        EBCL_errPrint("Could not store global option string.");
        free(copyData);
        return -1;
    }
    free(copyData);
    return 0;
}

int EBCL_globOptGetString(ebcl_GlobOptKey_t key, char **str) {
    size_t len = 0;
    if (EBCL_globOptGet(key, &len, sizeof(size_t)) == -1 || len == 0) {
        EBCL_errPrint("Could not get global option string.");
        return -1;
    }
    char *temp = malloc(len + sizeof(size_t));
    if (temp == NULL) {
        EBCL_errnoPrint("Could not allocate memory for temporary string");
        return -1;
    }
    if (EBCL_globOptGet(key, temp, len + sizeof(size_t)) == -1) {
        EBCL_errPrint("Could not get global option string.");
        free(temp);
        return -1;
    }
    *str = malloc(len);
    if (*str == NULL) {
        EBCL_errnoPrint("Could not allocate memory for output.");
        free(temp);
        return -1;
    }
    memcpy(*str, temp + sizeof(size_t), len);
    free(temp);
    return 0;
}

void EBCL_globOptDestroy(void) {
    if (pthread_mutex_lock(&EBCL_optLock) == -1) {
        EBCL_errnoPrint("Could not wait for global option array mutex lock during deinitialization.");
        return;
    }

    for (ebcl_GlobOptKey_t i = EBCL_GLOBOPT_START; i < EBCL_GLOBOPT_END; i++) {
        if (EBCL_globOptArr[i] != NULL) {
            free(EBCL_globOptArr[i]);
        }
    }
    pthread_mutex_unlock(&EBCL_optLock);
}

