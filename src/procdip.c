/**
 * @file procdip.c
 * @brief Implementation of the Process Dispatcher.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "procdip.h"

#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>

#include "logio.h"

#ifndef SYS_gettid
#error "SYS_gettid unavailable on this system"
#endif

/** Macro wrapper for the gettid syscall in case glibc is not new enough to contain one itself **/
#define gettid() ((pid_t)syscall(SYS_gettid))

/** Struct wrapper for arguments to dispatchThreadFunc **/
typedef struct ebcl_DispThrArgs_t {
    ebcl_TaskDB_t *ctx;    ///< The TaskDB context to update on task state changes.
    const ebcl_Task_t *t;  ///< The task to run.
} ebcl_DispThrArgs_t;

/** Mutex to guard #EBCL_waitInhibit **/
static pthread_mutex_t EBCL_waitInhibitLock = PTHREAD_MUTEX_INITIALIZER;
/** Condition variable to signal threads waiting for #EBCL_waitInhibit to become `false`. **/
static pthread_cond_t EBCL_waitInhibitCond = PTHREAD_COND_INITIALIZER;
/** If true, all terminated child processes will be kept around as zombies (see EBCL_blockOnWaitInhibit()). **/
static bool EBCL_waitInhibit = false;

/**
 * Function to be started as a pthread from EBCL_procDispatchThread().
 *
 * Takes care of process spawning/waiting and ebcl_TaskDB_t status updates.
 *
 * @param args  See ebcl_DispThrArgs_t.
 */
static void *EBCL_dispatchThreadFunc(void *args);
/**
 * Builds the standard environment for a new task.
 *
 * Currently, this contains only the required task name for sd_notify().
 *
 * Memory is allocated for the string. When no longer needed, the string should be freed.
 *
 * @param taskEnv   An array of char pointers of size 2 or more.
 * @param taskName  The task name to add to the environment.
 *
 * @return 0 on success, -1 on error
 */
static int EBCL_buildEnv(char **taskEnv, const char *taskName);

/**
 * Block calling thread until #EBCL_waitInhibit becomes false.
 *
 * Called by dispatcher threads to delay reaping of zombie processes if needed.
 *
 * @return 0 on success, -1 on error
 */
static int EBCL_blockOnWaitInhibit(void);
/**
 * Reap a zombie process.
 *
 * Will call blockOnWaitInhibit() internally.
 *
 * @param pid  The PID of the process to wait for.
 *
 * @return 0 on success, -1 on error
 */
static int EBCL_reapPid(pid_t pid);

int EBCL_procDispatchSpawnFunc(ebcl_TaskDB_t *ctx, const ebcl_Task_t *t) {
    pthread_t dispatchThread;
    pthread_attr_t dispatchThreadAttr;
    ebcl_DispThrArgs_t *threadArgs = malloc(sizeof(ebcl_DispThrArgs_t));
    if (threadArgs == NULL) {
        EBCL_errnoPrint("Could not allocate memory for thread arguments. Meant to create thread for task \'%s\'.",
                        t->name);
        return -1;
    }
    threadArgs->ctx = ctx;
    threadArgs->t = t;

    if ((errno = pthread_attr_init(&dispatchThreadAttr)) != 0) {
        EBCL_errnoPrint("Could not initialize pthread attributes. Meant to create thread for task \'%s\'.", t->name);
        free(threadArgs);
        return -1;
    }

    if ((errno = pthread_attr_setdetachstate(&dispatchThreadAttr, PTHREAD_CREATE_DETACHED)) != 0) {
        EBCL_errnoPrint("Could not set PTHREAD_CREATE_DETACHED attribute. Meant to create thread for task \'%s\'.",
                        t->name);
        goto fail;
    }

    if ((errno = pthread_attr_setstacksize(&dispatchThreadAttr, EBCL_PROC_DISPATCH_THREAD_STACK_SIZE)) != 0) {
        EBCL_errnoPrint("Could not set pthread stack size to %lu. Meant to create thread for task \'%s\'.",
                        EBCL_PROC_DISPATCH_THREAD_STACK_SIZE, t->name);
        goto fail;
    }

    EBCL_dbgInfoPrint("Creating new thread for Task \'%s\'.", t->name);
    if ((errno = pthread_create(&dispatchThread, &dispatchThreadAttr, EBCL_dispatchThreadFunc, threadArgs)) != 0) {
        EBCL_errnoPrint("Could not create pthread for task \'%s\'.", t->name);
        goto fail;
    }

    pthread_attr_destroy(&dispatchThreadAttr);
    return 0;
fail:
    pthread_attr_destroy(&dispatchThreadAttr);
    free(threadArgs);
    return -1;
}

static void *EBCL_dispatchThreadFunc(void *args) {
    ebcl_DispThrArgs_t *a = (ebcl_DispThrArgs_t *)args;
    ebcl_TaskDB_t *ctx = a->ctx;
    const ebcl_Task_t *t = a->t;
    ebcl_Task_t *tCopy = NULL;
    char *taskEnv[2] = {NULL, NULL};
    pid_t threadId = gettid();
    pid_t pid = -1;

    EBCL_dbgInfoPrint("(TID: %d) New thread started.", threadId);

    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        EBCL_errnoPrint("(TID: %d) Could not queue up for mutex lock.", threadId);
        goto threadExit;
    }

    if (EBCL_taskDup(&tCopy, t) == -1) {
        EBCL_errPrint("(TID: %d) Could not get duplicate of Task to spawn.", threadId);
        pthread_mutex_unlock(&ctx->lock);
        goto threadExit;
    }

    pthread_mutex_unlock(&ctx->lock);
    EBCL_dbgInfoPrint("(TID: %d) Will spawn Task \'%s\'.", threadId, tCopy->name);

    if (EBCL_buildEnv(taskEnv, tCopy->name) == -1) {
        EBCL_errPrint("(TID: %d) Could not build new environment for task \'%s\'.", threadId, tCopy->name);
        goto threadExit;
    }

    for (size_t i = 0; i < tCopy->cmdsSize; i++) {
        pid = fork();
        if (pid == -1) {
            EBCL_errnoPrint("(TID: %d) Could not fork new process for command %zu of Task \'%s\'", threadId, i,
                            tCopy->name);
            goto threadExit;
        }
        if (pid == 0) {  // child process
            if (execve(tCopy->cmds[i].argv[0], tCopy->cmds[i].argv, taskEnv) == -1) {
                EBCL_errnoPrint("(PID: %d) Could not exec into \'%s\'.", getpid(), tCopy->cmds[i].argv[0]);
                free(args);
                EBCL_freeTask(tCopy);
                exit(EXIT_FAILURE);
            }
        }
        // parent process
        EBCL_infoPrint("(TID: %d) Started new process %d for command %zu of Task \'%s\' (\'%s\').", threadId, pid, i,
                       tCopy->name, tCopy->cmds[i].argv[0]);
        if (i == 0) {
            if (EBCL_taskDBSetTaskState(ctx, EBCL_TASK_STATE_RUNNING, tCopy->name) == -1) {
                EBCL_errPrint("(TID: %d) Could not set state of Task \'%s\' to running.", threadId, tCopy->name);
                goto threadExit;
            }
            if (EBCL_taskDBSetTaskPID(ctx, pid, tCopy->name) == -1) {
                EBCL_errPrint("(TID: %d) Could not set PID of Task \'%s\' to %d.", threadId, tCopy->name, pid);
                goto threadExit;
            }
            char depEvent[sizeof(EBCL_TASK_EVENT_RUNNING)] = EBCL_TASK_EVENT_RUNNING;
            ebcl_TaskDep_t spawnDep = {tCopy->name, depEvent};
            if (EBCL_taskDBFulfillDep(ctx, &spawnDep) == -1) {
                EBCL_errPrint("(TID: %d) Could not fulfill dependency %s:%s.", threadId, spawnDep.name, spawnDep.event);
                goto threadExit;
            }
            EBCL_dbgInfoPrint("(TID: %d) Dependency \'%s:%s\' fulfilled.", threadId, spawnDep.name, spawnDep.event);
        }

        int wret;
        siginfo_t status;
        // Check if process has exited, but leave zombie.
        do {
            wret = waitid(P_PID, pid, &status, WEXITED | WNOWAIT);
        } while (wret != 0 && errno == EINTR);

        if (wret != 0 || status.si_code != CLD_EXITED || status.si_status != 0) {
            // There was some error, either Crinit-internal or the task returned an error code or the task was killed
            if (errno) {
                EBCL_errnoPrint("(TID: %d) Failed to wait for Task \'%s\' (PID %d).", threadId, tCopy->name, pid);
                goto threadExit;
            } else if (status.si_code == CLD_EXITED) {
                EBCL_infoPrint("(TID: %d) Task \'%s\' (PID %d) returned error code %d.", threadId, tCopy->name, pid,
                               status.si_status);
            } else {
                EBCL_infoPrint("(TID: %d) Task \'%s\' (PID %d) failed.", threadId, tCopy->name, pid);
            }

            if (EBCL_taskDBSetTaskState(ctx, EBCL_TASK_STATE_FAILED, tCopy->name) == -1) {
                EBCL_errPrint("(TID: %d) Could not set state of Task \'%s\' to failed.", threadId, tCopy->name);
                goto threadExit;
            }
            if (EBCL_taskDBSetTaskPID(ctx, -1, tCopy->name) == -1) {
                EBCL_errPrint("(TID: %d) Could not reset PID of failed Task \'%s\' to -1.", threadId, tCopy->name);
                goto threadExit;
            }
            // Reap zombie of failed command.
            if (EBCL_reapPid(pid) == -1) {
                EBCL_errnoPrint("(TID: %d) Could not reap zombie for task \'%s\'.", threadId, tCopy->name);
            }

            char depEvent[sizeof(EBCL_TASK_EVENT_FAILED)] = EBCL_TASK_EVENT_FAILED;
            ebcl_TaskDep_t failDep = {tCopy->name, depEvent};
            if (EBCL_taskDBFulfillDep(ctx, &failDep) == -1) {
                EBCL_errPrint("(TID: %d) Could not fulfill dependency %s:%s.", threadId, failDep.name, failDep.event);
            }
            EBCL_dbgInfoPrint("(TID: %d) Dependency \'%s:%s\' fulfilled.", threadId, failDep.name, failDep.event);
            goto threadExit;
        }

        // command of task has returned successfully
        if (EBCL_taskDBSetTaskPID(ctx, -1, tCopy->name) == -1) {
            EBCL_errPrint("(TID: %d) Could not reset PID of Task \'%s\' to -1.", threadId, tCopy->name);
            goto threadExit;
        }
        // Reap zombie of successful command.
        if (EBCL_reapPid(pid) == -1) {
            EBCL_errnoPrint("(TID: %d) Could not reap zombie for task \'%s\'.", threadId, tCopy->name);
        }
    }

    // chain of commands is done successfully
    EBCL_infoPrint("(TID: %d) Task \'%s\' done.", threadId, tCopy->name);
    if (EBCL_taskDBSetTaskState(ctx, EBCL_TASK_STATE_DONE, tCopy->name) == -1) {
        EBCL_errPrint("(TID: %d) Could not set state of Task \'%s\' to done.", threadId, tCopy->name);
        goto threadExit;
    }
    char depEvent[sizeof(EBCL_TASK_EVENT_DONE)] = EBCL_TASK_EVENT_DONE;
    ebcl_TaskDep_t doneDep = {tCopy->name, depEvent};
    if (EBCL_taskDBFulfillDep(ctx, &doneDep) == -1) {
        EBCL_errPrint("(TID: %d) Could not fulfill dependency %s:%s.", threadId, doneDep.name, doneDep.event);
    }
    EBCL_dbgInfoPrint("(TID: %d) Dependency \'%s:%s\' fulfilled.", threadId, doneDep.name, doneDep.event);

threadExit:
    EBCL_freeTask(tCopy);
    free(args);
    free(taskEnv[0]);
    return NULL;
}

int EBCL_setInhibitWait(bool inh) {
    int ret = 0;
    errno = pthread_mutex_lock(&EBCL_waitInhibitLock);
    if (errno != 0) {
        EBCL_errnoPrint("Could not lock on mutex.");
        return -1;
    }
    EBCL_waitInhibit = inh;
    if (!inh) {
        errno = pthread_cond_broadcast(&EBCL_waitInhibitCond);
        if (errno != 0) {
            ret = -1;
            EBCL_errnoPrint("Could not broadcast on condition variable.");
        }
    }
    if (pthread_mutex_unlock(&EBCL_waitInhibitLock)) {
        ret = -1;
        EBCL_errnoPrint("Could not unlock mutex.");
    }
    return ret;
}

static int EBCL_buildEnv(char **taskEnv, const char *taskName) {
    if (taskEnv == NULL || taskName == NULL) {
        EBCL_errPrint("Pointer arguments must not be NULL.");
        return -1;
    }
    // Add task name for sd_notify()
    size_t envPrefixLen = strlen(EBCL_CRINIT_ENV_NOTIFY_NAME "=");
    size_t envSuffixLen = strlen(taskName);
    size_t envSize = envPrefixLen + envSuffixLen + 1;
    taskEnv[0] = calloc(envSize, sizeof(char));
    if (taskEnv[0] == NULL) {
        EBCL_errnoPrint("Could not allocate %zu Bytes of memory for environment variable \'%s\'.", envSize,
                        EBCL_CRINIT_ENV_NOTIFY_NAME);
        taskEnv[1] = NULL;
        return -1;
    }
    memcpy(taskEnv[0], EBCL_CRINIT_ENV_NOTIFY_NAME "=", envPrefixLen);
    memcpy(taskEnv[0] + envPrefixLen, taskName, envSuffixLen);
    taskEnv[0][envPrefixLen + envSuffixLen] = '\0';
    taskEnv[1] = NULL;
    return 0;
}

static int EBCL_blockOnWaitInhibit(void) {
    errno = pthread_mutex_lock(&EBCL_waitInhibitLock);
    if (errno != 0) {
        EBCL_errnoPrint("Could not lock on mutex.");
        return -1;
    }
    while (EBCL_waitInhibit) {
        errno = pthread_cond_wait(&EBCL_waitInhibitCond, &EBCL_waitInhibitLock);
        if (errno != 0) {
            EBCL_errnoPrint("Could not wait on condition variable.");
            return -1;
        }
    }
    if (pthread_mutex_unlock(&EBCL_waitInhibitLock)) {
        EBCL_errnoPrint("Could not unlock mutex.");
        return -1;
    }
    return 0;
}

static int EBCL_reapPid(pid_t pid) {
    if (EBCL_blockOnWaitInhibit() == -1) {
        EBCL_errPrint("Could not block on wait inhibition condition.");
        return -1;
    }
    do {
        waitpid(pid, NULL, 0);
    } while (errno == EINTR);
    if (errno != 0) {
        EBCL_errnoPrint("Could not reap zombie for PID \'%d\'.", pid);
        return -1;
    }
    return 0;
}

