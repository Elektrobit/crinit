/**
 * @file crinit-sdefs.h
 * @brief Definitions shared between crinit's public and internal APIs.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __CRINIT_SDEFS_H__
#define __CRINIT_SDEFS_H__

/** Default path to Crinit's AF_UNIX communication socket. **/
#define EBCL_CRINIT_SOCKFILE "/run/crinit/crinit.sock"

/** The name/key of the environment variable Crinit passes to child processes for sd_notify(). */
#define EBCL_CRINIT_ENV_NOTIFY_NAME "CRINIT_TASK_NAME"

typedef unsigned long ebcl_TaskState_t;    ///< Type to store Task state bitmask.
#define EBCL_TASK_STATE_STARTING (1 << 0)  ///< Task state bitmask indicating the task currently spawns a new process.
#define EBCL_TASK_STATE_RUNNING (1 << 1)   ///< Bitmask indicating the task has spawned a process and is running.
#define EBCL_TASK_STATE_DONE (1 << 2)      ///< Bitmask indicating a task has finished without error.
#define EBCL_TASK_STATE_FAILED (1 << 3)    ///< Bitmask indicating a task has finished with an error code.

#endif /* __CRINIT_SDEFS_H__ */
