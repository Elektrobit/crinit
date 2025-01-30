// SPDX-License-Identifier: MIT
/**
 * @file elos-common.h
 * @brief Implementation file related to elos connection.
 */
#include "elos-common.h"

#include <dlfcn.h>
#include <errno.h>

#include "confparse.h"
#include "globopt.h"
#include "logio.h"

/**
 * Elos connection data and callbacks.
 */
static crinitElosVirtualTable_t crinitElosVTable;

/** Mutex protecting elos vtable initialization **/
static pthread_mutex_t crinitElosVTableLock = PTHREAD_MUTEX_INITIALIZER;

/**
 * Fetches a single symbol from the elos client shared library.
 *
 * @param lp Pointer to the elos shared library
 * @param symbolName Name of the symbol to be fetched
 * @param symbol Function pointer to be assigned
 * @return Returns 1 on success, 0 otherwise.
 */
static inline int crinitElosdepFetchElosSymbol(void *lp, const char *symbolName, void **symbol) {
    char *err;

    /* Clear any existing error */
    dlerror();

    *(void **)(symbol) = dlsym(lp, symbolName);
    if ((err = dlerror()) != NULL) {
        crinitErrPrint("Failed to load '%s' from %s: %s.", symbolName, LIBELOS_SO_FILENAME, err);
        dlclose(lp);
        return 0;
    }

    return 1;
}

int crinitElosInit(void) {
    int res = -1;
    void *lp;

    if ((errno = pthread_mutex_lock(&crinitElosVTableLock)) != 0) {
        crinitErrnoPrint("Failed to lock elos vtable.");
        return -1;
    }

    /* Return if the vtable has already been initialized */
    if (crinitElosVTable.connect != NULL) {
        res = 0;
        goto exit;
    }

    crinitDbgInfoPrint("Loading elos connection parameters.");
    if (crinitGlobOptGet(CRINIT_GLOBOPT_ELOS_SERVER, &crinitElosVTable.elosServer) == -1) {
        crinitDbgInfoPrint("No elos server ip configured in global options - using default.");
        crinitElosVTable.elosServer = strdup(CRINIT_CONFIG_DEFAULT_ELOS_SERVER);
        if (crinitElosVTable.elosServer == NULL) {
            crinitErrnoPrint("Failed to copy elos default server ip.");
            goto exit;
        }
    }

    if (crinitGlobOptGet(CRINIT_GLOBOPT_ELOS_PORT, &crinitElosVTable.elosPort) == -1) {
        crinitErrPrint("No elos server port configured in global options - using default.");
        crinitElosVTable.elosPort = CRINIT_CONFIG_DEFAULT_ELOS_PORT;
    }

    crinitDbgInfoPrint("Using elos connection parameters %s:%d.", crinitElosVTable.elosServer,
                       crinitElosVTable.elosPort);

    lp = dlopen(LIBELOS_SO_FILENAME, RTLD_NOW | RTLD_LOCAL);
    if (!lp) {
        crinitErrPrint("Failed to load dynamic library %s: %s.", LIBELOS_SO_FILENAME, dlerror());
        goto err_options;
    }

    res = crinitElosdepFetchElosSymbol(lp, "elosConnectTcpip", (void **)&crinitElosVTable.connect) &&
          crinitElosdepFetchElosSymbol(lp, "elosGetVersion", (void **)&crinitElosVTable.getVersion) &&
          crinitElosdepFetchElosSymbol(lp, "elosEventSubscribe", (void **)&crinitElosVTable.eventSubscribe) &&
          crinitElosdepFetchElosSymbol(lp, "elosEventUnsubscribe", (void **)&crinitElosVTable.eventUnsubscribe) &&
          crinitElosdepFetchElosSymbol(lp, "elosEventPublish", (void **)&crinitElosVTable.eventPublish) &&
          crinitElosdepFetchElosSymbol(lp, "elosEventQueueRead", (void **)&crinitElosVTable.eventQueueRead) &&
          crinitElosdepFetchElosSymbol(lp, "safuVecGetLast", (void **)&crinitElosVTable.eventVecGetLast) &&
          crinitElosdepFetchElosSymbol(lp, "elosEventVectorDelete", (void **)&crinitElosVTable.eventVectorDelete) &&
          crinitElosdepFetchElosSymbol(lp, "elosDisconnect", (void **)&crinitElosVTable.disconnect);

    if (res) {
        res = 0;
        goto exit;
    }

err_options:
    free(crinitElosVTable.elosServer);

exit:
    if ((errno = pthread_mutex_unlock(&crinitElosVTableLock)) != 0) {
        crinitErrnoPrint("Failed to unlock elos vtable.");
        return -1;
    }

    return res;
}

crinitElosVirtualTable_t *crinitElosGetVTable(void) {
    return &crinitElosVTable;
}

int crinitElosDisconnect(crinitElosSession_t *session, pthread_mutex_t *sessionLock) {
    int res = SAFU_RESULT_OK;

    if (session != NULL) {
        if ((errno = pthread_mutex_lock(sessionLock)) != 0) {
            crinitErrnoPrint("Failed to lock elos session.");
            return SAFU_RESULT_FAILED;
        }

        res = crinitElosVTable.disconnect(session);
        if (res != SAFU_RESULT_OK) {
            crinitErrPrint("Failed to disconnect from elosd.");
        }

        if ((errno = pthread_mutex_unlock(sessionLock)) != 0) {
            crinitErrnoPrint("Failed to unlock elos session.");
            res = SAFU_RESULT_FAILED;
        }
    }

    return res;
}
