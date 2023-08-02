// SPDX-License-Identifier: MIT
/**
 * @file thrpool.c
 * @brief Implementations of a generic worker thread pool. Used by the notification/service interface to handle socket
 * communication.
 */
#include "thrpool.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "logio.h"

/**
 * Grow the given thread pool to the given new size.
 *
 * Gets called by the dryPoolWatchdog() if not enough threads are idle/available.
 *
 * @param ctx      The thread pool to grow.
 * @param newSize  The new size of the thread pool, must be larger than the old size.
 *
 * @return 0 on success, -1 on error
 */
static int crinitThreadPoolGrow(crinitThreadPool_t *ctx, size_t newSize);
/**
 * Thread function to monitor the current load of the thread pool and grow its size if needed.
 *
 * To be started as a pthread running alongside the worker threads. Will monitor the given thread pool via
 * crinitThreadPool_t::threadAvail and crinitThreadPool_t::threadAvailChanged.
 *
 * A "dry pool" condition is detected and the thread pool grown if 90% or more of the pool's worker threads are
 * currently unavailable.
 *
 * @param thrpool  The crinitThreadPool_t to monitor.
 */
static void *crinitDryPoolWatchdog(void *thrpool);

/**
 * Function macro to calculate threshold value at and below which point the pool is considerd "dry".
 *
 * @param poolSize  The current total size of the worker thread pool.
 *
 * @return  The threshold value.
 */
#define crinitDryPoolThreshold(poolSize) ((poolSize) / 10)

int crinitThreadPoolInit(crinitThreadPool_t *ctx, size_t initialSize, void *(*threadFunc)(void *), const void *thrArgs,
                        size_t thrArgsSize) {
    if (ctx == NULL) {
        crinitErrPrint("Given ThreadPool context must not be NULL.");
        return -1;
    }
    ctx->threadAvail = 0;
    ctx->poolSize = 0;
    ctx->threadFunc = NULL;
    ctx->thrArgs = NULL;
    ctx->thrArgsSize = 0;

    if ((errno = pthread_mutex_init(&ctx->lock, NULL)) != 0) {
        crinitErrnoPrint("Could not initialize thread pool mutex.");
        return -1;
    }

    if (initialSize == 0) {
        initialSize = CRINIT_THREADPOOL_DEFAULT_INITIAL_SIZE;
    }

    ctx->poolSizeIncrement = initialSize;

    ctx->thrArgs = malloc(thrArgsSize);
    if (ctx->thrArgs == NULL) {
        crinitErrnoPrint("Could not allocate memory for thread arguments.");
        goto fail;
    }
    ctx->thrArgsSize = thrArgsSize;
    memcpy(ctx->thrArgs, thrArgs, thrArgsSize);
    ctx->threadFunc = threadFunc;

    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock on thread pool.");
        goto fail;
    }

    pthread_attr_t thrAttrs;
    if ((errno = pthread_attr_init(&thrAttrs)) != 0) {
        crinitErrnoPrint("Could not initialize pthread attributes.");
        pthread_mutex_unlock(&ctx->lock);
        goto fail;
    }
    if ((errno = pthread_attr_setdetachstate(&thrAttrs, PTHREAD_CREATE_DETACHED)) != 0) {
        crinitErrnoPrint("Could not set PTHREAD_CREATE_DETACHED attribute.");
        pthread_mutex_unlock(&ctx->lock);
        pthread_attr_destroy(&thrAttrs);
        goto fail;
    }

    if ((errno = pthread_attr_setstacksize(&thrAttrs, CRINIT_THREADPOOL_THREAD_STACK_SIZE)) != 0) {
        crinitErrnoPrint("Could not set pthread stack size to %d.", CRINIT_THREADPOOL_THREAD_STACK_SIZE);
        pthread_mutex_unlock(&ctx->lock);
        pthread_attr_destroy(&thrAttrs);
        goto fail;
    }

    crinitDbgInfoPrint("Initializing thread pool.");
    if ((errno = pthread_create(&ctx->dryPoolWdRef, &thrAttrs, crinitDryPoolWatchdog, ctx)) != 0) {
        crinitErrnoPrint("Could not create dry thread pool watchdog thread.");
        pthread_mutex_unlock(&ctx->lock);
        pthread_attr_destroy(&thrAttrs);
        goto fail;
    }
    crinitDbgInfoPrint("Created dry pool watchdog.");
    pthread_mutex_unlock(&ctx->lock);
    pthread_attr_destroy(&thrAttrs);

    if (crinitThreadPoolGrow(ctx, initialSize) == -1) {
        crinitErrPrint("Could not create worker threads.");
        goto fail;
    }
    crinitDbgInfoPrint("Created %zu worker threads.", initialSize);
    return 0;

fail:
    free(ctx->thrArgs);
    ctx->thrArgs = NULL;
    ctx->thrArgsSize = 0;
    return -1;
}

int crinitThreadPoolThreadBusyCallback(crinitThreadPool_t *ctx) {
    if (ctx == NULL) {
        crinitErrPrint("The given thread pool context must not be NULL.");
        return -1;
    }
    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock on thread pool.");
        return -1;
    }
    ctx->threadAvail--;
    if (ctx->threadAvail <= crinitDryPoolThreshold(ctx->poolSize)) {
        pthread_cond_signal(&ctx->threadAvailChanged);
    }
    pthread_mutex_unlock(&ctx->lock);
    return 0;
}

int crinitThreadPoolThreadAvailCallback(crinitThreadPool_t *ctx) {
    if (ctx == NULL) {
        crinitErrPrint("The given thread pool context must not be NULL.");
        return -1;
    }
    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock on thread pool.");
        return -1;
    }
    ctx->threadAvail++;

    pthread_mutex_unlock(&ctx->lock);
    return 0;
}

static int crinitThreadPoolGrow(crinitThreadPool_t *ctx, size_t newSize) {
    if (ctx == NULL) {
        crinitErrPrint("The given thread pool context must not be NULL.");
        return -1;
    }

    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock on thread pool.");
        return -1;
    }

    if (newSize <= ctx->poolSize) {
        crinitErrPrint("New size of thread pool (%zu) must be larger than the old size (%zu).", newSize, ctx->poolSize);
        pthread_mutex_unlock(&ctx->lock);
        return -1;
    }

    pthread_attr_t thrAttrs;
    if ((errno = pthread_attr_init(&thrAttrs)) != 0) {
        crinitErrnoPrint("Could not initialize pthread attributes.");
        pthread_mutex_unlock(&ctx->lock);
        return -1;
    }
    if ((errno = pthread_attr_setdetachstate(&thrAttrs, PTHREAD_CREATE_DETACHED)) != 0) {
        crinitErrnoPrint("Could not set PTHREAD_CREATE_DETACHED attribute.");
        pthread_mutex_unlock(&ctx->lock);
        pthread_attr_destroy(&thrAttrs);
        return -1;
    }
    if ((errno = pthread_attr_setstacksize(&thrAttrs, CRINIT_THREADPOOL_THREAD_STACK_SIZE)) != 0) {
        crinitErrnoPrint("Could not set pthread stack size to %d.", CRINIT_THREADPOOL_THREAD_STACK_SIZE);
        pthread_mutex_unlock(&ctx->lock);
        pthread_attr_destroy(&thrAttrs);
        return -1;
    }

    pthread_t thr;
    size_t oldSize = ctx->poolSize;
    for (size_t i = oldSize; i < newSize; i++) {
        if ((errno = pthread_create(&thr, &thrAttrs, ctx->threadFunc, ctx->thrArgs)) != 0) {
            crinitErrnoPrint("Could not create thread pool pthread number %zu.", i);
            pthread_mutex_unlock(&ctx->lock);
            pthread_attr_destroy(&thrAttrs);
            return -1;
        } else {
            crinitDbgInfoPrint("Created worker thread %zu.", i);
            ctx->poolSize++;
            ctx->threadAvail++;
        }
    }

    pthread_mutex_unlock(&ctx->lock);
    pthread_attr_destroy(&thrAttrs);
    return 0;
}

static void *crinitDryPoolWatchdog(void *thrpool) {
    crinitThreadPool_t *ctx = (crinitThreadPool_t *)thrpool;

    if (ctx == NULL) {
        crinitErrPrint("The given thread pool context must not be NULL.");
        return NULL;
    }
    crinitDbgInfoPrint("Dry pool watchdog thread started.");
    while (true) {
        if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
            crinitErrnoPrint("Could not queue up for mutex lock on thread pool.");
            return NULL;
        }
        pthread_cond_wait(&ctx->threadAvailChanged, &ctx->lock);
        if (ctx->threadAvail <= crinitDryPoolThreshold(ctx->poolSize)) {
            size_t newSize = ctx->poolSize + ctx->poolSizeIncrement;
            pthread_mutex_unlock(&ctx->lock);
            if (crinitThreadPoolGrow(ctx, newSize) == -1) {
                crinitErrPrint("Could not grow thread pool.");
            }
        } else {
            pthread_mutex_unlock(&ctx->lock);
        }
    }

    return NULL;
}
