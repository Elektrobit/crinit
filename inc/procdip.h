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
#define EBCL_PROC_DISPATCH_THREAD_STACK_SIZE 8 * PTHREAD_STACK_MIN
/**
 * The name/key of the environment variable Crinit passes to child processes for sd_notify().
 */
#define EBCL_CRINIT_ENV_NOTIFY_NAME "CRINIT_TASK_NAME"

/**
 * Process dispatcher function to spawn a task that is ready.
 *
 * Will create a new thread to handle process spawning of task \a t and status updates of \a ctx. The function will
 * return successfully if the thread has been created without error. The thread is created in a detached state so no
 * further management action is necessary.
 *
 * @return 0 on success, -1 otherwise
 */
int EBCL_procDispatchSpawnFunc(ebcl_TaskDB *ctx, const ebcl_Task *t);

#endif /* __PROCDIP_H__ */
