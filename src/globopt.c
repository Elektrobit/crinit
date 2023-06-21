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

#define crinitGlobOptSetErrPrint(keyStr) crinitErrPrint("Could not set default value for global option '%s'.", (keyStr))

/** Array to hold pointers to the global option values, one pointer per global option. **/
static void *crinitGlobOptArr[CRINIT_GLOBOPT_END - CRINIT_GLOBOPT_START] = {NULL};
/** Mutex to synchronize access to globOptArr **/
static pthread_mutex_t crinitOptLock = PTHREAD_MUTEX_INITIALIZER;

int crinitGlobOptInitDefault(void) {
    for (crinitGlobOptKey_t i = CRINIT_GLOBOPT_START; i < CRINIT_GLOBOPT_END; i++) {
        switch (i) {
            case CRINIT_GLOBOPT_DEBUG: {
                bool def = CRINIT_CONFIG_DEFAULT_DEBUG;
                if (crinitGlobOptSetBoolean(i, &def) == -1) {
                    crinitGlobOptSetErrPrint(CRINIT_CONFIG_KEYSTR_DEBUG);
                    return -1;
                }
            } break;
            case CRINIT_GLOBOPT_TASKDIR:
                if (crinitGlobOptSetString(i, CRINIT_CONFIG_DEFAULT_TASKDIR) == -1) {
                    crinitGlobOptSetErrPrint(CRINIT_CONFIG_KEYSTR_TASKDIR);
                    return -1;
                }
                break;
            case CRINIT_GLOBOPT_INCLDIR:
                if (crinitGlobOptSetString(i, CRINIT_CONFIG_DEFAULT_INCLDIR) == -1) {
                    crinitGlobOptSetErrPrint(CRINIT_CONFIG_KEYSTR_INCLDIR);
                    return -1;
                }
                break;
            case CRINIT_GLOBOPT_INCL_SUFFIX:
                if (crinitGlobOptSetString(i, CRINIT_CONFIG_DEFAULT_INCL_SUFFIX) == -1) {
                    crinitGlobOptSetErrPrint(CRINIT_CONFIG_KEYSTR_INCL_SUFFIX);
                    return -1;
                }
                break;
            case CRINIT_GLOBOPT_SHDGRACEP: {
                unsigned long long def = CRINIT_CONFIG_DEFAULT_SHDGRACEP;
                if (crinitGlobOptSetUnsignedLL(i, &def) == -1) {
                    crinitGlobOptSetErrPrint(CRINIT_CONFIG_KEYSTR_SHDGRACEP);
                    return -1;
                }
            } break;
            case CRINIT_GLOBOPT_USE_SYSLOG: {
                bool def = CRINIT_CONFIG_DEFAULT_USE_SYSLOG;
                if (crinitGlobOptSetBoolean(i, &def) == -1) {
                    crinitGlobOptSetErrPrint(CRINIT_CONFIG_KEYSTR_USE_SYSLOG);
                    return -1;
                }
            } break;
            case CRINIT_GLOBOPT_ENV: {
                crinitEnvSet_t init = {NULL, 0, 0};
                if (crinitGlobOptSet(i, &init, sizeof(crinitEnvSet_t)) == -1) {
                    crinitGlobOptSetErrPrint(CRINIT_CONFIG_KEYSTR_ENV_SET);
                    return -1;
                }
            } break;
            case CRINIT_GLOBOPT_START:
            default:
                if (crinitGlobOptSet(i, NULL, 0) == -1) {
                    crinitErrPrint("Could not set unknown global option to default NULL pointer.");
                    return -1;
                }
            case CRINIT_GLOBOPT_END:
                break;
        }
    }
    return 0;
}

int crinitGlobOptSet(crinitGlobOptKey_t key, const void *val, size_t sz) {
    if ((errno = pthread_mutex_lock(&crinitOptLock)) == -1) {
        crinitErrnoPrint("Could not wait for global option array mutex lock.");
        return -1;
    }
    size_t idx = (size_t)key;
    if (crinitGlobOptArr[idx] != NULL) {
        free(crinitGlobOptArr[idx]);
    }
    if (val == NULL || sz == 0) {
        crinitGlobOptArr[idx] = NULL;
    } else {
        crinitGlobOptArr[idx] = malloc(sz);
        if (crinitGlobOptArr[idx] == NULL) {
            crinitErrPrint("Could not allocate memory for global option.");
            pthread_mutex_unlock(&crinitOptLock);
            return -1;
        }
        memcpy(crinitGlobOptArr[idx], val, sz);
    }
    pthread_mutex_unlock(&crinitOptLock);
    return 0;
}

int crinitGlobOptGet(crinitGlobOptKey_t key, void *val, size_t sz) {
    if (val == NULL || sz == 0) {
        crinitErrPrint("Return value pointer must not be NULL and at least 1 Byte must be read.");
        return -1;
    }
    if ((errno = pthread_mutex_lock(&crinitOptLock)) == -1) {
        crinitErrnoPrint("Could not wait for global option array mutex lock.");
        return -1;
    }
    size_t idx = (size_t)key;
    if (crinitGlobOptArr[idx] == NULL) {
        crinitErrPrint("Could not read global option as it is uninitialized.");
        pthread_mutex_unlock(&crinitOptLock);
        return -1;
    }
    memcpy(val, crinitGlobOptArr[idx], sz);
    pthread_mutex_unlock(&crinitOptLock);
    return 0;
}

int crinitGlobOptSetString(crinitGlobOptKey_t key, const char *str) {
    if (str == NULL) {
        crinitErrPrint("Input string must not be NULL.");
        return -1;
    }
    size_t len = strlen(str) + 1;
    char *copyData = malloc(len + sizeof(size_t));
    if (copyData == NULL) {
        crinitErrnoPrint("Could not allocate memory for global option string.");
        return -1;
    }
    memcpy(copyData, &len, sizeof(size_t));
    memcpy(copyData + sizeof(size_t), str, len);
    if (crinitGlobOptSet(key, copyData, len + sizeof(size_t)) == -1) {
        crinitErrPrint("Could not store global option string.");
        free(copyData);
        return -1;
    }
    free(copyData);
    return 0;
}

int crinitGlobOptGetString(crinitGlobOptKey_t key, char **str) {
    size_t len = 0;
    if (crinitGlobOptGet(key, &len, sizeof(size_t)) == -1 || len == 0) {
        crinitErrPrint("Could not get global option string.");
        return -1;
    }
    char *temp = malloc(len + sizeof(size_t));
    if (temp == NULL) {
        crinitErrnoPrint("Could not allocate memory for temporary string");
        return -1;
    }
    if (crinitGlobOptGet(key, temp, len + sizeof(size_t)) == -1) {
        crinitErrPrint("Could not get global option string.");
        free(temp);
        return -1;
    }
    *str = malloc(len);
    if (*str == NULL) {
        crinitErrnoPrint("Could not allocate memory for output.");
        free(temp);
        return -1;
    }
    memcpy(*str, temp + sizeof(size_t), len);
    free(temp);
    return 0;
}

int crinitGlobOptSetEnvSet(const crinitEnvSet_t *es) {
    crinitEnvSet_t tgt;
    if (crinitGlobOptGet(CRINIT_GLOBOPT_ENV, &tgt, sizeof(crinitEnvSet_t)) == -1) {
        crinitErrPrint("Could not retrieve current global environment set.");
        return -1;
    }

    if ((errno = pthread_mutex_lock(&crinitOptLock)) == -1) {
        crinitErrnoPrint("Could not wait for global option array mutex lock.");
        return -1;
    }
    if (crinitEnvSetDestroy(&tgt) == -1) {
        crinitErrPrint("Could not free old environment set during update of set.");
        pthread_mutex_unlock(&crinitOptLock);
        return -1;
    }
    if (crinitEnvSetDup(&tgt, es) == -1) {
        crinitErrPrint("Could not duplicate new environment set for use as a global option.");
    }

    free(crinitGlobOptArr[CRINIT_GLOBOPT_ENV]);
    crinitGlobOptArr[CRINIT_GLOBOPT_ENV] = malloc(sizeof(crinitEnvSet_t));
    if (crinitGlobOptArr[CRINIT_GLOBOPT_ENV] == NULL) {
        crinitErrPrint("Could not allocate memory for global option.");
        pthread_mutex_unlock(&crinitOptLock);
        return -1;
    }
    memcpy(crinitGlobOptArr[CRINIT_GLOBOPT_ENV], &tgt, sizeof(crinitEnvSet_t));
    pthread_mutex_unlock(&crinitOptLock);
    return 0;
}

int crinitGlobOptGetEnvSet(crinitEnvSet_t *es) {
    crinitEnvSet_t temp;
    if (crinitGlobOptGet(CRINIT_GLOBOPT_ENV, &temp, sizeof(crinitEnvSet_t)) == -1) {
        crinitErrPrint("Could not retrieve global environment set.");
        return -1;
    }
    if ((errno = pthread_mutex_lock(&crinitOptLock)) == -1) {
        crinitErrnoPrint("Could not wait for global option array mutex lock.");
        return -1;
    }
    if (crinitEnvSetDup(es, &temp) == -1) {
        crinitErrPrint("Could not duplicate environment set during retrieval from global options.");
        pthread_mutex_unlock(&crinitOptLock);
        return -1;
    }
    pthread_mutex_unlock(&crinitOptLock);
    return 0;
}

void crinitGlobOptDestroy(void) {
    if (pthread_mutex_lock(&crinitOptLock) == -1) {
        crinitErrnoPrint("Could not wait for global option array mutex lock during deinitialization.");
        return;
    }

    for (crinitGlobOptKey_t i = CRINIT_GLOBOPT_START; i < CRINIT_GLOBOPT_END; i++) {
        if (crinitGlobOptArr[i] != NULL) {
            if (i == CRINIT_GLOBOPT_ENV) {
                crinitEnvSetDestroy(crinitGlobOptArr[i]);
            }
            free(crinitGlobOptArr[i]);
        }
    }
    pthread_mutex_unlock(&crinitOptLock);
}
