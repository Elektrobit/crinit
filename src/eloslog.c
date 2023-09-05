// SPDX-License-Identifier: MIT
/**
 * @file eloslog.c
 * @brief Implementation of elos event handling.
 */
#include "eloslog.h"

#include <errno.h>
#include <pthread.h>
#include <safu/common.h>
#include <safu/ringbuffer.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "confparse.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define CRINIT_MACHINE_ID_FILE "/etc/machine-id"
#define CRINIT_MACHINE_ID_LENGTH 32
#define CRINIT_MACHINE_ID_FORMAT "%" STR(CRINIT_MACHINE_ID_LENGTH) "s"

static bool crinitUseElos = CRINIT_CONFIG_DEFAULT_USE_ELOS;  ///< Specifies if we should send elos events.

/**
 * Thread context of the eloslog main thread.
 */
static struct crinitElosEventThread {
    pthread_t threadId;            ///< Thread identifier
    bool elosStarted;              ///< Wether or not an initial conenction to elos has been established
    crinitElosSession_t *session;  ///< Elos session handle
} crinitTinfo;

/** Mutex synchronizing elos connection **/
static pthread_mutex_t crinitEloslogSessionLock = PTHREAD_MUTEX_INITIALIZER;

/** Ring buffer of elos event messages */
static safuRingBuffer_t crinitElosEventBuffer;

/*
static safuResultE_t crinitDeleteEvent(crinitElosEvent_t *event) {
    safuResultE_t result = SAFU_RESULT_FAILED;
    if (event != NULL) {
        free(event->hardwareid);
        free(event->payload);
        result = SAFU_RESULT_OK;
    }
    return result;
}
*/

static inline int crinitFetchHWId(char *hwId) {
    FILE *fp = NULL;

    if (hwId == NULL) {
        return -1;
    }

    fp = fopen(CRINIT_MACHINE_ID_FILE, "r");
    if (fp == NULL) {
        crinitErrnoPrint("Failed to open machine id file %s.\n", CRINIT_MACHINE_ID_FILE);
        return -1;
    }

    int res = fscanf(fp, CRINIT_MACHINE_ID_FORMAT, hwId);
    if (res != 1 || ferror(fp)) {
        crinitErrnoPrint("Failed to read machine id from %s.\n", CRINIT_MACHINE_ID_FILE);
        fclose(fp);
        return -1;
    }

    fclose(fp);

    return 0;
}

static inline int crinitPublishEvent(void const *element, void const *data) {
    CRINIT_PARAM_UNUSED(data);

    int res;
    char hwId[CRINIT_MACHINE_ID_LENGTH + 1] = {0};
    crinitElosEvent_t *event = *(crinitElosEvent_t **)element;
    crinitElosEventSource_t eventSource = {.appName = "crinit", .pid = 1};
    event->source = eventSource;

    crinitDbgInfoPrint("Publishing event to elos: '%s'", event->payload);

    res = crinitFetchHWId(&hwId[0]);
    if (res != 0) {
        crinitErrPrint("Failed to fetch hardware id - continue.");
    } else {
        event->hardwareid = &hwId[0];
    }

    return crinitElosTryExec(crinitTinfo.session, &crinitEloslogSessionLock, crinitElosGetVTable()->eventPublish,
                             "Failed to publish crinit event.", crinitTinfo.session, event);
}

static void *crinitEloslogEventListener(void *arg) {
    CRINIT_PARAM_UNUSED(arg);

    int res;
    safuVec_t events = {0};
    const char *version;

    crinitTinfo.elosStarted = true;

    res = crinitElosTryExec(crinitTinfo.session, &crinitEloslogSessionLock, crinitElosGetVTable()->getVersion,
                            "Failed to request elos version.", crinitTinfo.session, &version);
    if (res == SAFU_RESULT_OK) {
        crinitInfoPrint("Connected to elosd running version: %s", version);
    }

    while (1) {
        res = safuRingBufferRead(&crinitElosEventBuffer, &events, NULL);
        if (res == SAFU_RESULT_OK) {
            safuVecIterate(&events, crinitPublishEvent, NULL);
            safuVecFree(&events);
        } else {
            crinitErrPrint("Initializing elos event buffer failed.");
            break;
        }
    }

    crinitElosDisconnect(crinitTinfo.session, &crinitEloslogSessionLock);

    return NULL;
}

int crinitElosLog(crinitElosSeverityE_t severity, int messageCode, const char *format, ...) {
    safuResultE_t result;
    size_t bytes;

    /* The plugin has not been activated */
    if (!crinitUseElos) {
        return 0;
    }

    crinitElosEvent_t *event = calloc(1, sizeof(*event));
    if (event == NULL) {
        crinitErrnoPrint("Failed to allocate memory for the elos event.");
        return -1;
    }

    va_list argp, argp2;
    va_start(argp, format);
    va_copy(argp2, argp);
    bytes = vsnprintf(NULL, 0, format, argp) + 1;
    event->payload = calloc(1, bytes);
    if (event->payload == NULL) {
        crinitErrnoPrint("Failed to allocate memory for the elos event message.");
        va_end(argp);
        va_end(argp2);
        free(event);
        return SAFU_RESULT_FAILED;
    }

    vsnprintf(event->payload, bytes, format, argp2);
    va_end(argp2);

    event->date.tv_sec = time(NULL);
    event->date.tv_nsec = 0;
    event->severity = severity;
    event->messageCode = messageCode;

    crinitDbgInfoPrint("Enqueuing elos event: '%s'", event->payload);

    result = safuRingBufferWrite(&crinitElosEventBuffer, event);
    if (result != SAFU_RESULT_OK) {
        crinitErrPrint("Writing to elos event buffer failed.");
        free(event);
    }

    return result;
}

int crinitEloslogInit(void) {
    int res;

    crinitInfoPrint("Initializing elos event logging.");

    /* If deleting entries, the ring buffer can just use free() */
    const safuRingBufferParam_t ringBufferParams = {
        /* .deleteEntries = true, */
        .elements = CRINIT_ELOSLOG_EVENT_LIMIT,
    };

    res = safuRingBufferInitialize(&crinitElosEventBuffer, &ringBufferParams);
    if (res != SAFU_RESULT_OK) {
        crinitErrPrint("Initializing elos event buffer failed.");
        return -1;
    }

    return 0;
}

int crinitEloslogActivate(bool sl) {
    int res;

    if (sl && !crinitUseElos) {
        if (crinitElosInit() != 0) {
            crinitErrPrint("Failed to load elos interface.");
            return -1;
        }

        pthread_attr_t attrs;
        res = pthread_attr_init(&attrs);
        if (res != 0) {
            crinitErrPrint("Could not initialize pthread attributes.");
            return -1;
        }

        res = pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
        if (res != 0) {
            crinitErrPrint("Could not set PTHREAD_CREATE_DETACHED attribute.");
            pthread_attr_destroy(&attrs);
            return -1;
        }

        res = pthread_attr_setstacksize(&attrs, CRINIT_THREADPOOL_THREAD_STACK_SIZE);
        if (res != 0) {
            crinitErrPrint("Could not set pthread stack size to %d.", CRINIT_THREADPOOL_THREAD_STACK_SIZE);
            pthread_attr_destroy(&attrs);
            return -1;
        }

        crinitDbgInfoPrint("Starting elos event handler thread.");
        res = pthread_create(&crinitTinfo.threadId, &attrs, crinitEloslogEventListener, NULL);
        if (res != 0) {
            crinitErrPrint("Could not create elos event handler thread.");
            pthread_attr_destroy(&attrs);
            return -1;
        }

        res = pthread_attr_destroy(&attrs);
        if (res != 0) {
            crinitErrPrint("Failed to free thread attributes.");
            return -1;
        }
    } else if (!sl && crinitUseElos) {
        // TODO: Kill thread.
        crinitUseElos = sl;
    }

    return 0;
}
