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

#define EBCL_globOptSetErrPrint(keyStr) EBCL_errPrint("Could not set default value for global option '%s'.", (keyStr))

/** Array to hold pointers to the global option values, one pointer per global option. **/
static void *EBCL_globOptArr[EBCL_GLOBOPT_END - EBCL_GLOBOPT_START] = {NULL};
/** Mutex to synchronize access to globOptArr **/
static pthread_mutex_t EBCL_optLock = PTHREAD_MUTEX_INITIALIZER;

int EBCL_globOptInitDefault(void) {
    for (ebcl_GlobOptKey_t i = EBCL_GLOBOPT_START; i < EBCL_GLOBOPT_END; i++) {
        switch (i) {
            case EBCL_GLOBOPT_DEBUG: {
                bool def = EBCL_CONFIG_DEFAULT_DEBUG;
                if (EBCL_globOptSetBoolean(i, &def) == -1) {
                    EBCL_globOptSetErrPrint(EBCL_CONFIG_KEYSTR_DEBUG);
                    return -1;
                }
            } break;
            case EBCL_GLOBOPT_TASKDIR:
                if (EBCL_globOptSetString(i, EBCL_CONFIG_DEFAULT_TASKDIR) == -1) {
                    EBCL_globOptSetErrPrint(EBCL_CONFIG_KEYSTR_TASKDIR);
                    return -1;
                }
                break;
            case EBCL_GLOBOPT_INCLDIR:
                if (EBCL_globOptSetString(i, EBCL_CONFIG_DEFAULT_INCLDIR) == -1) {
                    EBCL_globOptSetErrPrint(EBCL_CONFIG_KEYSTR_INCLDIR);
                    return -1;
                }
                break;
            case EBCL_GLOBOPT_INCL_SUFFIX:
                if (EBCL_globOptSetString(i, EBCL_CONFIG_DEFAULT_INCL_SUFFIX) == -1) {
                    EBCL_globOptSetErrPrint(EBCL_CONFIG_KEYSTR_INCL_SUFFIX);
                    return -1;
                }
                break;
            case EBCL_GLOBOPT_SHDGRACEP: {
                unsigned long long def = EBCL_CONFIG_DEFAULT_SHDGRACEP;
                if (EBCL_globOptSetUnsignedLL(i, &def) == -1) {
                    EBCL_globOptSetErrPrint(EBCL_CONFIG_KEYSTR_SHDGRACEP);
                    return -1;
                }
            } break;
            case EBCL_GLOBOPT_USE_SYSLOG: {
                bool def = EBCL_CONFIG_DEFAULT_USE_SYSLOG;
                if (EBCL_globOptSetBoolean(i, &def) == -1) {
                    EBCL_globOptSetErrPrint(EBCL_CONFIG_KEYSTR_USE_SYSLOG);
                    return -1;
                }
            } break;
            case EBCL_GLOBOPT_ENV: {
                ebcl_EnvSet_t init = {NULL, 0, 0};
                if (EBCL_globOptSet(i, &init, sizeof(ebcl_EnvSet_t)) == -1) {
                    EBCL_globOptSetErrPrint(EBCL_CONFIG_KEYSTR_ENV_SET);
                    return -1;
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

int EBCL_globOptSetEnvSet(const ebcl_EnvSet_t *es) {
    ebcl_EnvSet_t tgt;
    if (EBCL_globOptGet(EBCL_GLOBOPT_ENV, &tgt, sizeof(ebcl_EnvSet_t)) == -1) {
        EBCL_errPrint("Could not retrieve current global environment set.");
        return -1;
    }

    if ((errno = pthread_mutex_lock(&EBCL_optLock)) == -1) {
        EBCL_errnoPrint("Could not wait for global option array mutex lock.");
        return -1;
    }
    if (EBCL_envSetDestroy(&tgt) == -1) {
        EBCL_errPrint("Could not free old environment set during update of set.");
        pthread_mutex_unlock(&EBCL_optLock);
        return -1;
    }
    if (EBCL_envSetDup(&tgt, es) == -1) {
        EBCL_errPrint("Could not duplicate new environment set for use as a global option.");
    }

    free(EBCL_globOptArr[EBCL_GLOBOPT_ENV]);
    EBCL_globOptArr[EBCL_GLOBOPT_ENV] = malloc(sizeof(ebcl_EnvSet_t));
    if (EBCL_globOptArr[EBCL_GLOBOPT_ENV] == NULL) {
        EBCL_errPrint("Could not allocate memory for global option.");
        pthread_mutex_unlock(&EBCL_optLock);
        return -1;
    }
    memcpy(EBCL_globOptArr[EBCL_GLOBOPT_ENV], &tgt, sizeof(ebcl_EnvSet_t));
    pthread_mutex_unlock(&EBCL_optLock);
    return 0;
}

int EBCL_globOptGetEnvSet(ebcl_EnvSet_t *es) {
    ebcl_EnvSet_t temp;
    if (EBCL_globOptGet(EBCL_GLOBOPT_ENV, &temp, sizeof(ebcl_EnvSet_t)) == -1) {
        EBCL_errPrint("Could not retrieve global environment set.");
        return -1;
    }
    if ((errno = pthread_mutex_lock(&EBCL_optLock)) == -1) {
        EBCL_errnoPrint("Could not wait for global option array mutex lock.");
        return -1;
    }
    if (EBCL_envSetDup(es, &temp) == -1) {
        EBCL_errPrint("Could not duplicate environment set during retrieval from global options.");
        pthread_mutex_unlock(&EBCL_optLock);
        return -1;
    }
    pthread_mutex_unlock(&EBCL_optLock);
    return 0;
}

void EBCL_globOptDestroy(void) {
    if (pthread_mutex_lock(&EBCL_optLock) == -1) {
        EBCL_errnoPrint("Could not wait for global option array mutex lock during deinitialization.");
        return;
    }

    for (ebcl_GlobOptKey_t i = EBCL_GLOBOPT_START; i < EBCL_GLOBOPT_END; i++) {
        if (EBCL_globOptArr[i] != NULL) {
            if (i == EBCL_GLOBOPT_ENV) {
                EBCL_envSetDestroy(EBCL_globOptArr[i]);
            }
            free(EBCL_globOptArr[i]);
        }
    }
    pthread_mutex_unlock(&EBCL_optLock);
}
