// SPDX-License-Identifier: MIT
/**
 * @file thrpool.h
 * @brief Header defining a generic worker thread pool. Used by the notification/service interface to handle socket
 * communication.
 */
#ifndef __THRPOOL_H__
#define __THRPOOL_H__

#include <limits.h>
#include <pthread.h>

/**
 * Structure holding a worker thread pool.
 */
typedef struct crinitThreadPool_t {
    size_t poolSize;                    ///< Current size of the pool.
    size_t poolSizeIncrement;           ///< How many new threads to create if the pool runs dry.
    size_t threadAvail;                 ///< Number of available worker threads.
    pthread_mutex_t lock;               ///< Mutex protecting changes to the thread pool structure.
    pthread_cond_t threadAvailChanged;  ///< Condition variable signalled if crinitThreadPool_t::threadAvail is changed.
    pthread_t dryPoolWdRef;  ///< Reference to the dry pool watchdog thread, see dryPoolWatchdog() in thrpool.c.

    void *(*threadFunc)(void *args);  ///< Thread function for all worker threads.
    void *thrArgs;                    ///< Arguments to the thread function.
    size_t thrArgsSize;               ///< Number of arguments in crinitThreadPool_t::thrArgs.
} crinitThreadPool_t;

/**
 * Default initial size (in number of threads) of the thread pool
 */
#define CRINIT_THREADPOOL_DEFAULT_INITIAL_SIZE 8
/**
 * Stack size of the threads within the thread pool.
 */
#define CRINIT_THREADPOOL_THREAD_STACK_SIZE (PTHREAD_STACK_MIN + 112 * 1024)

/**
 * Initialize an crinitThreadPool_t.
 *
 * @param ctx          The crinitThreadPool_t to initialize.
 * @param initialSize  Initial size (in number of threads) of the pool.
 * @param threadFunc   Worker thread function to use.
 * @param thrArgs      Arguments to the worker thread function. Will be copied and saved in case more threads need to be
 * started by dryPoolWatchdog.
 * @param thrArgsSize  Size (Bytes) of arguments to the worker thread function.
 *
 * @return 0 on success, -1 otherwise
 */
int crinitThreadPoolInit(crinitThreadPool_t *ctx, size_t initialSize, void *(*threadFunc)(void *), const void *thrArgs,
                        size_t thrArgsSize);

/**
 * Callback to be used by the worker thread function signalling it is busy/unavailable.
 *
 * @param ctx  The crinitThreadPool_t context.
 *
 * @return 0 on success, -1 otherwise
 */
int crinitThreadPoolThreadBusyCallback(crinitThreadPool_t *ctx);
/**
 * Callback to be used by the worker thread function signalling it is idle/available.
 *
 * @param ctx  The crinitThreadPool_t context.
 *
 * @return 0 on success, -1 otherwise
 */
int crinitThreadPoolThreadAvailCallback(crinitThreadPool_t *ctx);

#endif /* __THRPOOL_H__ */
