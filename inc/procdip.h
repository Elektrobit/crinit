/**
 * @file procdip.h
 * @brief Header related to the Process Dispatcher.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __PROCDIP_H__
#define __PROCDIP_H__

#include <limits.h>

#include "taskdb.h"

/**
 * Stack size for process dispatch threads.
 */
#define CRINIT_PROC_DISPATCH_THREAD_STACK_SIZE (PTHREAD_STACK_MIN + 112 * 1024)

/**
 * Process dispatcher function to spawn a task that is ready.
 *
 * Will create a new thread to handle process spawning of task \a t and status updates of \a ctx. The function will
 * return successfully if the thread has been created without error. The thread is created in a detached state so no
 * further management action is necessary.
 *
 * @return 0 on success, -1 otherwise
 */
int crinitProcDispatchSpawnFunc(crinitTaskDB_t *ctx, const crinitTask_t *t);

/**
 * Turn waiting for child processes on or off.
 *
 * If \a inh is set to true, the process dispatch threads will block before waiting for a child process until
 * waiting is reactivated, leaving terminated child processes as zombies for the time being.
 *
 * @param inh  Set true to inhibit waiting or false to reactivate waiting.
 *
 * @return 0 on success, -1 on error
 */
int crinitSetInhibitWait(bool inh);

#endif /* __PROCDIP_H__ */
