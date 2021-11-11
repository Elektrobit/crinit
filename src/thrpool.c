/**
 * @file thrpool.c
 * @brief Implementations of a generic worker thread pool. Used by the notification/service interface to handle socket
 * communication.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
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
static int threadPoolGrow(ebcl_ThreadPool *ctx, size_t newSize);
/**
 * Thread function to monitor the current load of the thread pool and grow its size if needed.
 *
 * To be started as a pthread running alongside the worker threads. Will monitor the given thread pool via
 * ebcl_ThreadPool::threadAvail and ebcl_ThreadPool::threadAvailChanged.
 *
 * A "dry pool" condition is detected and the thread pool grown if 90% or more of the pool's worker threads are
 * currently unavailable.
 *
 * @param thrpool  The ebcl_ThreadPool to monitor.
 */
static void *dryPoolWatchdog(void *thrpool);

/**
 * Function macro to calculate threshold value at and below which point the pool is considerd "dry".
 *
 * @param poolSize  The current total size of the worker thread pool.
 *
 * @return  The threshold value.
 */
#define DRY_POOL_THRESHOLD(poolSize) ((poolSize) / 10)

int EBCL_threadPoolInit(ebcl_ThreadPool *ctx, size_t initialSize, void *(*threadFunc)(void *), const void *thrArgs,
                        size_t thrArgsSize) {
    if (ctx == NULL) {
        EBCL_errPrint("Given ThreadPool context must not be NULL.");
        return -1;
    }
    ctx->threadAvail = 0;
    ctx->poolSize = 0;
    ctx->pool = NULL;
    ctx->threadFunc = NULL;
    ctx->thrArgs = NULL;
    ctx->thrArgsSize = 0;

    if ((errno = pthread_mutex_init(&ctx->lock, NULL)) != 0) {
        EBCL_errnoPrint("Could not initialize thread pool mutex.");
        return -1;
    }

    if (initialSize == 0) {
        initialSize = EBCL_THREADPOOL_DEFAULT_INITIAL_SIZE;
    }

    ctx->pool = malloc(initialSize * sizeof(pthread_t));
    if (ctx->pool == NULL) {
        EBCL_errnoPrint("Could not allocate memory for thread pool of size %lu.", initialSize);
        goto fail;
    }
    ctx->poolSize = initialSize;
    ctx->poolSizeIncrement = initialSize;
    ctx->threadAvail = initialSize;

    ctx->thrArgs = malloc(thrArgsSize);
    if (ctx->thrArgs == NULL) {
        EBCL_errnoPrint("Could not allocate memory for thread arguments.");
        goto fail;
    }
    ctx->thrArgsSize = thrArgsSize;
    memcpy(ctx->thrArgs, thrArgs, thrArgsSize);
    ctx->threadFunc = threadFunc;

    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        EBCL_errnoPrint("Could not queue up for mutex lock on thread pool.");
        goto fail;
    }

    pthread_attr_t thrAttrs;
    if ((errno = pthread_attr_init(&thrAttrs)) != 0) {
        EBCL_errnoPrint("Could not initialize pthread attributes.");
        pthread_mutex_unlock(&ctx->lock);
        goto fail;
    }
    if ((errno = pthread_attr_setdetachstate(&thrAttrs, PTHREAD_CREATE_DETACHED)) != 0) {
        EBCL_errnoPrint("Could not set PTHREAD_CREATE_DETACHED attribute.");
        pthread_mutex_unlock(&ctx->lock);
        pthread_attr_destroy(&thrAttrs);
        goto fail;
    }

    if ((errno = pthread_attr_setstacksize(&thrAttrs, EBCL_THREADPOOL_THREAD_STACK_SIZE)) != 0) {
        EBCL_errnoPrint("Could not set pthread stack size to %lu.", EBCL_THREADPOOL_THREAD_STACK_SIZE);
        pthread_mutex_unlock(&ctx->lock);
        pthread_attr_destroy(&thrAttrs);
        goto fail;
    }

    EBCL_dbgInfoPrint("Initializing thread pool.");
    if ((errno = pthread_create(&ctx->dryPoolWdRef, &thrAttrs, dryPoolWatchdog, ctx)) != 0) {
        EBCL_errnoPrint("Could not create dry thread pool watchdog thread.");
        pthread_mutex_unlock(&ctx->lock);
        pthread_attr_destroy(&thrAttrs);
        goto fail;
    }
    EBCL_dbgInfoPrint("Created dry pool watchdog.");

    for (size_t i = 0; i < ctx->poolSize; i++) {
        if ((errno = pthread_create(&ctx->pool[i], &thrAttrs, ctx->threadFunc, ctx->thrArgs)) != 0) {
            EBCL_errnoPrint("Could not create thread pool pthread number %lu.", i);
            pthread_cancel(ctx->dryPoolWdRef);
            pthread_mutex_unlock(&ctx->lock);
            pthread_attr_destroy(&thrAttrs);
            goto fail;
        }
        EBCL_dbgInfoPrint("Created worker thread %lu. Function pointer %p.", i, ctx->threadFunc);
    }

    pthread_mutex_unlock(&ctx->lock);
    pthread_attr_destroy(&thrAttrs);
    return 0;

fail:
    free(ctx->pool);
    ctx->pool = NULL;
    free(ctx->thrArgs);
    ctx->thrArgs = NULL;
    ctx->thrArgsSize = 0;
    pthread_mutex_destroy(&ctx->lock);
    return -1;
}

int EBCL_threadPoolDestroy(ebcl_ThreadPool *ctx) {
    if (ctx == NULL) {
        EBCL_errPrint("Could not destroy thread pool because the pointer given to it was NULL.");
        return -1;
    }
    pthread_cancel(ctx->dryPoolWdRef);
    free(ctx->pool);
    ctx->poolSize = 0;
    ctx->threadAvail = 0;
    if ((errno = pthread_mutex_destroy(&ctx->lock)) != 0) {
        EBCL_errnoPrint("Could not destroy mutex during destruction of thread pool.");
        return -1;
    }
    return 0;
}

int EBCL_threadPoolThreadBusyCallback(ebcl_ThreadPool *ctx) {
    if (ctx == NULL) {
        EBCL_errPrint("The given thread pool context must not be NULL.");
        return -1;
    }
    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        EBCL_errnoPrint("Could not queue up for mutex lock on thread pool.");
        return -1;
    }
    ctx->threadAvail--;
    if (ctx->threadAvail <= DRY_POOL_THRESHOLD(ctx->poolSize)) {
        pthread_cond_signal(&ctx->threadAvailChanged);
    }
    pthread_mutex_unlock(&ctx->lock);
    return 0;
}

int EBCL_threadPoolThreadAvailCallback(ebcl_ThreadPool *ctx) {
    if (ctx == NULL) {
        EBCL_errPrint("The given thread pool context must not be NULL.");
        return -1;
    }
    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        EBCL_errnoPrint("Could not queue up for mutex lock on thread pool.");
        return -1;
    }
    ctx->threadAvail++;

    pthread_mutex_unlock(&ctx->lock);
    return 0;
}

static int threadPoolGrow(ebcl_ThreadPool *ctx, size_t newSize) {
    if (ctx == NULL) {
        EBCL_errPrint("The given thread pool context must not be NULL.");
        return -1;
    }

    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        EBCL_errnoPrint("Could not queue up for mutex lock on thread pool.");
        return -1;
    }

    if (newSize <= ctx->poolSize) {
        EBCL_errPrint("New size of thread pool (%lu) must be larger than the old size (%lu).", newSize, ctx->poolSize);
        pthread_mutex_unlock(&ctx->lock);
        return -1;
    }

    pthread_t *newPool = realloc(ctx->pool, newSize * sizeof(pthread_t));
    if (newPool == NULL) {
        EBCL_errnoPrint("Could not allocate additional memory to grow the thread pool from size %lu to size %lu.",
                        ctx->poolSize, newSize);
        pthread_mutex_unlock(&ctx->lock);
        return -1;
    }

    pthread_attr_t thrAttrs;
    if ((errno = pthread_attr_init(&thrAttrs)) != 0) {
        EBCL_errnoPrint("Could not initialize pthread attributes.");
        pthread_mutex_unlock(&ctx->lock);
        return -1;
    }
    if ((errno = pthread_attr_setdetachstate(&thrAttrs, PTHREAD_CREATE_DETACHED)) != 0) {
        EBCL_errnoPrint("Could not set PTHREAD_CREATE_DETACHED attribute.");
        pthread_mutex_unlock(&ctx->lock);
        pthread_attr_destroy(&thrAttrs);
        return -1;
    }
    if ((errno = pthread_attr_setstacksize(&thrAttrs, EBCL_THREADPOOL_THREAD_STACK_SIZE)) != 0) {
        EBCL_errnoPrint("Could not set pthread stack size to %lu.", EBCL_THREADPOOL_THREAD_STACK_SIZE);
        pthread_mutex_unlock(&ctx->lock);
        pthread_attr_destroy(&thrAttrs);
        return -1;
    }

    for (size_t i = 0; i < ctx->poolSize; i++) {
        if ((errno = pthread_create(&ctx->pool[i], &thrAttrs, ctx->threadFunc, ctx->thrArgs)) != 0) {
            EBCL_errnoPrint("Could not create thread pool pthread number %lu.", i);
        } else {
            EBCL_dbgInfoPrint("Created worker thread %lu. Function pointer %p.", i, ctx->threadFunc);
        }
    }
    ctx->pool = newPool;
    ctx->poolSize = newSize;
    ctx->threadAvail = newSize;

    pthread_mutex_unlock(&ctx->lock);
    pthread_attr_destroy(&thrAttrs);
    return 0;
}

static void *dryPoolWatchdog(void *thrpool) {
    ebcl_ThreadPool *ctx = (ebcl_ThreadPool *)thrpool;

    if (ctx == NULL) {
        EBCL_errPrint("The given thread pool context must not be NULL.");
        return NULL;
    }
    EBCL_dbgInfoPrint("Dry pool watchdog thread started.");
    while (true) {
        if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
            EBCL_errnoPrint("Could not queue up for mutex lock on thread pool.");
            return NULL;
        }
        pthread_cond_wait(&ctx->threadAvailChanged, &ctx->lock);
        if (ctx->threadAvail <= DRY_POOL_THRESHOLD(ctx->poolSize)) {
            size_t newSize = ctx->poolSize + ctx->poolSizeIncrement;
            pthread_mutex_unlock(&ctx->lock);
            threadPoolGrow(ctx, newSize);
        } else {
            pthread_mutex_unlock(&ctx->lock);
        }
    }

    return NULL;
}

