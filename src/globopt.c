/**
 * @file globopt.c
 * @brief Implementation of global option storage.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "globopt.h"

#include <pthread.h>
#include <stdlib.h>

#include "common.h"
#include "logio.h"

/** Common error message for crinitGlobOptInitDefault(). **/
#define crinitGlobOptSetErrPrint(keyStr) crinitErrPrint("Could not set default value for global option '%s'.", (keyStr))

/**
 * Common boilerplate to acquire a lock on the global option storage.
 */
#define crinitGlobOptCommonLock()                                                   \
    if (crinitGlobOptBorrow() == NULL) {                                            \
        crinitErrPrint("Could not get exclusive access to global option storage."); \
        return -1;                                                                  \
    }

/**
 * Common boilerplate to release a lock on the global option storage.
 */
#define crinitGlobOptCommonUnlock()                                                     \
    if (crinitGlobOptRemit() == -1) {                                                   \
        crinitErrPrint("Could not release exclusive access to global option storage."); \
        return -1;                                                                      \
    }

/** Central global option storage. **/
static crinitGlobOptStore_t crinitGlobOpts;
/** Mutex to synchronize access to globOptArr **/
static pthread_mutex_t crinitOptLock = PTHREAD_MUTEX_INITIALIZER;

int crinitGlobOptInitDefault(void) {
    crinitGlobOptCommonLock();

    memset(&crinitGlobOpts, 0, sizeof(crinitGlobOpts));
    crinitGlobOpts.debug = CRINIT_CONFIG_DEFAULT_DEBUG;
    crinitGlobOpts.useSyslog = CRINIT_CONFIG_DEFAULT_USE_SYSLOG;
    crinitGlobOpts.shdGraceP = CRINIT_CONFIG_DEFAULT_SHDGRACEP;

    crinitGlobOpts.inclDir = strdup(CRINIT_CONFIG_DEFAULT_INCLDIR);
    if (crinitGlobOpts.inclDir == NULL) {
        crinitGlobOptSetErrPrint(CRINIT_CONFIG_KEYSTR_INCLDIR);
        crinitGlobOptDestroy();
        crinitGlobOptCommonUnlock();
        return -1;
    }

    crinitGlobOpts.inclSuffix = strdup(CRINIT_CONFIG_DEFAULT_INCL_SUFFIX);
    if (crinitGlobOpts.inclSuffix == NULL) {
        crinitGlobOptSetErrPrint(CRINIT_CONFIG_KEYSTR_INCL_SUFFIX);
        crinitGlobOptDestroy();

        crinitGlobOptCommonUnlock();
        return -1;
    }

    if (crinitEnvSetInit(&crinitGlobOpts.globEnv, CRINIT_ENVSET_INITIAL_SIZE, CRINIT_ENVSET_SIZE_INCREMENT) == -1) {
        crinitGlobOptSetErrPrint(CRINIT_CONFIG_KEYSTR_ENV_SET);
        crinitGlobOptDestroy();
        crinitGlobOptCommonUnlock();
        return -1;
    }

    crinitGlobOptCommonUnlock();
    return 0;
}

int crinitGlobOptSetString(size_t memberOffset, const char *val) {
    crinitNullCheck(-1, val);
    char *goptBase = (char *)&crinitGlobOpts;
    char **tgt = (char **)(goptBase + memberOffset);

    crinitGlobOptCommonLock();

    free(*tgt);
    *tgt = strdup(val);
    if (*tgt == NULL) {
        crinitErrnoPrint("Could not duplicate string to global option storage.");
        crinitGlobOptCommonUnlock();
        return -1;
    }

    crinitGlobOptCommonUnlock();
    return 0;
}

int crinitGlobOptGetString(size_t memberOffset, char **val) {
    crinitNullCheck(-1, val);
    char *goptBase = (char *)&crinitGlobOpts;
    char *src = *(char **)(goptBase + memberOffset);

    if (crinitGlobOptBorrow() == NULL) {
        crinitErrPrint("Could not geti exclusive access to global option storage.");
        return -1;
    }
    *val = strdup(src);
    if (*val == NULL) {
        crinitErrnoPrint("Could not duplicate string to global option storage.");
        crinitGlobOptRemit();
        return -1;
    }
    if (crinitGlobOptRemit() == -1) {
        crinitErrPrint("Could not release exclusive access to global option storage.");
        return -1;
    }
    return 0;
}

int crinitGlobOptSetBoolean(size_t memberOffset, bool val) {
    char *goptBase = (char *)&crinitGlobOpts;
    bool *tgt = (bool *)(goptBase + memberOffset);

    crinitGlobOptCommonLock();
    *tgt = val;
    crinitGlobOptCommonUnlock();

    return 0;
}

int crinitGlobOptGetBoolean(size_t memberOffset, bool *val) {
    crinitNullCheck(-1, val);
    char *goptBase = (char *)&crinitGlobOpts;

    crinitGlobOptCommonLock();
    *val = *(bool *)(goptBase + memberOffset);
    crinitGlobOptCommonUnlock();

    return 0;
}

int crinitGlobOptSetUnsignedLL(size_t memberOffset, unsigned long long val) {
    char *goptBase = (char *)&crinitGlobOpts;
    unsigned long long *tgt = (unsigned long long *)(goptBase + memberOffset);

    crinitGlobOptCommonLock();
    *tgt = val;
    crinitGlobOptCommonUnlock();

    return 0;
}

int crinitGlobOptGetUnsignedLL(size_t memberOffset, unsigned long long *val) {
    crinitNullCheck(-1, val);
    char *goptBase = (char *)&crinitGlobOpts;

    crinitGlobOptCommonLock();
    *val = *(unsigned long long *)(goptBase + memberOffset);
    crinitGlobOptCommonUnlock();

    return 0;
}

int crinitGlobOptSetEnvSet(size_t memberOffset, const crinitEnvSet_t *val) {
    crinitNullCheck(-1, val);
    char *goptBase = (char *)&crinitGlobOpts;
    crinitEnvSet_t *tgt = (crinitEnvSet_t *)(goptBase + memberOffset);

    crinitGlobOptCommonLock();

    if (crinitEnvSetDestroy(tgt) == -1) {
        crinitErrPrint("Could not free memory of global environment prior to overwriting it.");
        crinitGlobOptCommonUnlock();
        return -1;
    }
    if (crinitEnvSetDup(tgt, val) == -1) {
        crinitErrPrint("Could not duplicate environment set into global option storage.");
        crinitGlobOptCommonUnlock();
        return -1;
    }

    crinitGlobOptCommonUnlock();
    return 0;
}

int crinitGlobOptGetEnvSet(size_t memberOffset, crinitEnvSet_t *val) {
    crinitNullCheck(-1, val);
    char *goptBase = (char *)&crinitGlobOpts;
    crinitEnvSet_t *src = (crinitEnvSet_t *)(goptBase + memberOffset);

    crinitGlobOptCommonLock();

    if (crinitEnvSetDup(val, src) == -1) {
        crinitErrPrint("Could not duplicate environment set from global option storage.");
        crinitGlobOptCommonUnlock();
        return -1;
    }

    crinitGlobOptCommonUnlock();
    return 0;
}

crinitGlobOptStore_t *crinitGlobOptBorrow(void) {
    if ((errno = pthread_mutex_lock(&crinitOptLock)) == -1) {
        crinitErrnoPrint("Could not wait for global option array mutex lock.");
        return NULL;
    }
    return &crinitGlobOpts;
}

int crinitGlobOptRemit(void) {
    // This *could* be called from a thread which does not actually own the mutex, so we need to check if
    // pthread_mutex_unlock() fails.
    errno = pthread_mutex_unlock(&crinitOptLock);
    if (errno != 0) {
        crinitErrnoPrint("Could not unlock global option mutex.");
        return -1;
    }
    return 0;
}

void crinitGlobOptDestroy(void) {
    if (pthread_mutex_lock(&crinitOptLock) == -1) {
        crinitErrnoPrint("Could not wait for global option array mutex lock during deinitialization.");
        return;
    }
    free(crinitGlobOpts.inclDir);
    free(crinitGlobOpts.inclSuffix);
    crinitEnvSetDestroy(&crinitGlobOpts.globEnv);
    pthread_mutex_unlock(&crinitOptLock);
}
