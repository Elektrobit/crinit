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

#include "crinit.h"
#include "logio.h"

#ifndef SYS_gettid
#error "SYS_gettid unavailable on this system"
#endif

/** Macro wrapper for the gettid syscall in case glibc is not new enough to contain one itself **/
#define gettid() ((pid_t)syscall(SYS_gettid))

/** Struct wrapper for arguments to dispatchThreadFunc **/
typedef struct dispThrArgs {
    ebcl_TaskDB *ctx;    ///< The TaskDB context to update on task state changes.
    const ebcl_Task *t;  ///< The task to run.
} dispThrArgs;

/** POSIX variable declaration pointing to the current process environment **/
extern char **environ;

/**
 * Function to be started as a pthread from EBCL_procDispatchThread().
 *
 * Takes care of process spawning/waiting and ebcl_TaskDB status updates.
 *
 * @param args  See dispThrArgs.
 */
static void *dispatchThreadFunc(void *args);
/**
 * Builds a new environment array from #environ, adding the task name for sd_notify().
 *
 * Memory is allocated for the outer array and each string. When no longer needed, all pointers should be passed to
 * free() individually.
 *
 * @param newEnv    Return pointer for the new environment.
 * @param taskName  The task name to add to the environment.
 *
 * @return 0 on success, -1 on error
 */
static int buildEnv(char ***newEnv, const char *taskName);

int EBCL_procDispatchSpawnFunc(ebcl_TaskDB *ctx, const ebcl_Task *t) {
    pthread_t dispatchThread;
    pthread_attr_t dispatchThreadAttr;
    dispThrArgs *threadArgs = malloc(sizeof(dispThrArgs));
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
    if ((errno = pthread_create(&dispatchThread, &dispatchThreadAttr, dispatchThreadFunc, threadArgs)) != 0) {
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

static void *dispatchThreadFunc(void *args) {
    dispThrArgs *a = (dispThrArgs *)args;
    ebcl_TaskDB *ctx = a->ctx;
    const ebcl_Task *t = a->t;
    ebcl_Task *tCopy = NULL;
    char **taskEnv = NULL;
    pid_t threadId = gettid();

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

    if(buildEnv(&taskEnv, tCopy->name) == -1) {
        EBCL_errPrint("(TID: %d) Could not build new environment for task \'%s\'.", threadId, tCopy->name);
        goto threadExit;
    }

    for (size_t i = 0; i < tCopy->cmdsSize; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            EBCL_errnoPrint("(TID: %d) Could not fork new process for command %lu of Task \'%s\'", threadId, i,
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
        EBCL_infoPrint("(TID: %d) Started new process %d for command %lu of Task \'%s\' (\'%s\').", threadId, pid, i,
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
            ebcl_TaskDep spawnDep = {tCopy->name, depEvent};
            if (EBCL_taskDBFulfillDep(ctx, &spawnDep) == -1) {
                EBCL_errPrint("(TID: %d) Could not fulfill dependency %s:%s.", threadId, spawnDep.name, spawnDep.event);
                goto threadExit;
            }
            EBCL_dbgInfoPrint("(TID: %d) Dependency \'%s:%s\' fulfilled.", threadId, spawnDep.name, spawnDep.event);
        }
        errno = 0;
        int status = 0;
        do {
            waitpid(pid, &status, 0);
        } while (errno == EINTR);
        if (errno || !WIFEXITED(status) || WEXITSTATUS(status)) {
            if (WIFEXITED(status)) {
                EBCL_infoPrint("(TID: %d) Task \'%s\' (PID %d) returned error code %d.", threadId, tCopy->name, pid,
                               WEXITSTATUS(status));
            } else if (errno) {
                EBCL_errnoPrint("(TID: %d) Failed to wait for Task \'%s\' (PID %d).", threadId, tCopy->name, pid);
                goto threadExit;
            } else {
                EBCL_infoPrint("(TID: %d) Task \'%s\' (PID %d) failed.", threadId, tCopy->name, pid);
            }

            if (EBCL_taskDBSetTaskState(ctx, EBCL_TASK_STATE_FAILED, tCopy->name) == -1) {
                EBCL_errPrint("(TID: %d) Could not set state of Task \'%s\' to failed.", threadId, tCopy->name);
                goto threadExit;
            }
            char depEvent[sizeof(EBCL_TASK_EVENT_FAILED)] = EBCL_TASK_EVENT_FAILED;
            ebcl_TaskDep failDep = {tCopy->name, depEvent};
            if (EBCL_taskDBFulfillDep(ctx, &failDep) == -1) {
                EBCL_errPrint("(TID: %d) Could not fulfill dependency %s:%s.", threadId, failDep.name, failDep.event);
            }
            EBCL_dbgInfoPrint("(TID: %d) Dependency \'%s:%s\' fulfilled.", threadId, failDep.name, failDep.event);
            goto threadExit;
        }
    }

    EBCL_infoPrint("(TID: %d) Task \'%s\' done.", threadId, tCopy->name);
    if (EBCL_taskDBSetTaskState(ctx, EBCL_TASK_STATE_DONE, tCopy->name) == -1) {
        EBCL_errPrint("(TID: %d) Could not set state of Task \'%s\' to done.", threadId, tCopy->name);
        goto threadExit;
    }
    char depEvent[sizeof(EBCL_TASK_EVENT_DONE)] = EBCL_TASK_EVENT_DONE;
    ebcl_TaskDep doneDep = {tCopy->name, depEvent};
    if (EBCL_taskDBFulfillDep(ctx, &doneDep) == -1) {
        EBCL_errPrint("(TID: %d) Could not fulfill dependency %s:%s.", threadId, doneDep.name, doneDep.event);
    }
    EBCL_dbgInfoPrint("(TID: %d) Dependency \'%s:%s\' fulfilled.", threadId, doneDep.name, doneDep.event);

threadExit:
    EBCL_freeTask(tCopy);
    free(args);
    if(taskEnv != NULL) {
        char **pEnv = taskEnv;
        while(*pEnv != NULL) {
            free(*pEnv);
            pEnv++;
        }
    }
    free(taskEnv);
    return NULL;
}

static int buildEnv(char ***newEnv, const char *taskName) {
    size_t envCount = 0;
    char **pEnv = environ;
    // check size of current environment
    while (*pEnv != NULL) {
        envCount++;
        pEnv++;
    }
    // make space for one more
    envCount++;
    // allocate for envCount elements plus the terminating NULL pointer
    *newEnv = calloc(envCount+1, sizeof(char *));
    if (newEnv == NULL) {
        EBCL_errnoPrint("Could not allocate memory for new process environment.");
        return -1;
    }
    char **pNew = *newEnv;
    // copy everything
    for (size_t i = 0; i < envCount - 1; i++) {
        pNew[i] = strdup(environ[i]);
        if (pNew[i] == NULL) {
            EBCL_errnoPrint("Could not duplicate environment string \'%s\'.", environ[i]);
            return -1;
        }
    }
    // Add task name for sd_notify()
    size_t envPrefixLen = strlen(EBCL_CRINIT_ENV_NOTIFY_NAME "=");
    size_t envSuffixLen = strlen(taskName);
    size_t envSize = envPrefixLen + envSuffixLen + 1;
    pNew[envCount - 1] = calloc(envSize, sizeof(char));
    if (pNew[envCount - 1] == NULL) {
        EBCL_errnoPrint("Could not allocate %lu Bytes of memory for environment variable \'%s\'.", envSize,
                        EBCL_CRINIT_ENV_NOTIFY_NAME);
        return -1;
    }
    memcpy(pNew[envCount - 1], EBCL_CRINIT_ENV_NOTIFY_NAME "=", envPrefixLen);
    memcpy(pNew[envCount - 1]+envPrefixLen, taskName, envSuffixLen);
    pNew[envCount - 1][envPrefixLen + envSuffixLen] = '\0';
    pNew[envCount] = NULL;
    return 0;
}

