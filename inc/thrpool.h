/**
 * @file thrpool.h
 * @brief Header defining a generic worker thread pool. Used by the notification/service interface to handle socket
 * communication.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __THRPOOL_H__
#define __THRPOOL_H__

#include <limits.h>
#include <pthread.h>

/**
 * Structure holding a worker thread pool.
 */
typedef struct ebcl_ThreadPool {
    size_t poolSize;                    ///< Current size of the pool.
    size_t poolSizeIncrement;           ///< How many new threads to create if the pool runs dry.
    size_t threadAvail;                 ///< Number of available worker threads.
    pthread_mutex_t lock;               ///< Mutex protecting changes to the thread pool structure.
    pthread_cond_t threadAvailChanged;  ///< Condition variable signalled if ebcl_ThreadPool::threadAvail is changed.
    pthread_t dryPoolWdRef;  ///< Reference to the dry pool watchdog thread, see dryPoolWatchdog() in thrpool.c.

    void *(*threadFunc)(void *args);  ///< Thread function for all worker threads.
    void *thrArgs;                    ///< Arguments to the thread function.
    size_t thrArgsSize;               ///< Number of arguments in ebcl_ThreadPool::thrArgs.
} ebcl_ThreadPool;

/**
 * Default initial size (in number of threads) of the thread pool
 */
#define EBCL_THREADPOOL_DEFAULT_INITIAL_SIZE 8
/**
 * Stack size of the threads within the thread pool.
 */
#define EBCL_THREADPOOL_THREAD_STACK_SIZE (PTHREAD_STACK_MIN + 112 * 1024)

/**
 * Initialize an ebcl_ThreadPool.
 *
 * @param ctx          The ebcl_ThreadPool to initialize.
 * @param initialSize  Initial size (in number of threads) of the pool.
 * @param threadFunc   Worker thread function to use.
 * @param thrArgs      Arguments to the worker thread function. Will be copied and saved in case more threads need to be
 *                     started by dryPoolWatchdog.
 * @param thrArgsSize  Size (Bytes) of arguments to the worker thread function.
 *
 * @return 0 on success, -1 otherwise
 */
int EBCL_threadPoolInit(ebcl_ThreadPool *ctx, size_t initialSize, void *(*threadFunc)(void *), const void *thrArgs,
                        size_t thrArgsSize);

/**
 * Callback to be used by the worker thread function signalling it is busy/unavailable.
 *
 * @param ctx  The ebcl_ThreadPool context.
 *
 * @return 0 on success, -1 otherwise
 */
int EBCL_threadPoolThreadBusyCallback(ebcl_ThreadPool *ctx);
/**
 * Callback to be used by the worker thread function signalling it is idle/available.
 *
 * @param ctx  The ebcl_ThreadPool context.
 *
 * @return 0 on success, -1 otherwise
 */
int EBCL_threadPoolThreadAvailCallback(ebcl_ThreadPool *ctx);

#endif /* __THRPOOL_H__ */
