// SPDX-License-Identifier: MIT
/**
 * @file crinit-sdefs.h
 * @brief Definitions shared between crinit's public and internal APIs.
 */
#ifndef __CRINIT_SDEFS_H__
#define __CRINIT_SDEFS_H__

#include <sys/types.h>

/** Default path to Crinit's AF_UNIX communication socket. **/
#define CRINIT_SOCKFILE "/run/crinit/crinit.sock"

/** The name/key of the environment variable Crinit passes to child processes for sd_notify(). */
#define CRINIT_ENV_NOTIFY_NAME "CRINIT_TASK_NAME"

typedef unsigned long crinitTaskState_t;     ///< Type to store Task state bitmask.
#define CRINIT_TASK_STATE_LOADED (0 << 0)    ///< Task state bitmask indicating the task was loaded, but never ran.
#define CRINIT_TASK_STATE_STARTING (1 << 0)  ///< Task state bitmask indicating the task currently spawns a new process.
#define CRINIT_TASK_STATE_RUNNING (1 << 1)   ///< Bitmask indicating the task has spawned a process and is running.
#define CRINIT_TASK_STATE_DONE (1 << 2)      ///< Bitmask indicating a task has finished without error.
#define CRINIT_TASK_STATE_FAILED (1 << 3)    ///< Bitmask indicating a task has finished with an error code.
#define CRINIT_TASK_STATE_NOTIFIED (1 << 4)  ///< Bitmask indicating the state was reported through the sd_notify()-API.

/** Type to represent an entry in a task list. **/
typedef struct crinitTaskListEntry_t {
    char *name;               ///< Task name.
    pid_t pid;                ///< PID of currently running process subordinate to the task, if any.
    crinitTaskState_t state;  ///< Task state.
} crinitTaskListEntry_t;

/** Type to represent a list of tasks. **/
typedef struct crinitTaskList_t {
    size_t numTasks;               ///< Number of elements in the \a tasks array.
    crinitTaskListEntry_t *tasks;  ///< Array of task entries.
} crinitTaskList_t;

/** Type to represent the shutdown action crinit shall perform. **/
typedef enum crinitShutdownCmd_t {
    CRINIT_SHD_UNDEF = 0,     ///< undefined/error value
    CRINIT_SHD_POWEROFF = 1,  ///< perform a graceful shutdown
    CRINIT_SHD_REBOOT = 2     ///< perform a graceful reboot
} crinitShutdownCmd_t;

#endif /* __CRINIT_SDEFS_H__ */
