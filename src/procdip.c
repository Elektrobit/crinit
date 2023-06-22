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

#include <spawn.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "envset.h"
#include "logio.h"

#ifndef SYS_gettid
#error "SYS_gettid unavailable on this system"
#endif

/** Macro wrapper for the gettid syscall in case glibc is not new enough to contain one itself **/
#define crinitGettid() ((pid_t)syscall(SYS_gettid))

/** Struct wrapper for arguments to dispatchThreadFunc **/
typedef struct ebcl_DispThrArgs_t {
    crinitTaskDB_t *ctx;    ///< The TaskDB context to update on task state changes.
    const crinitTask_t *t;  ///< The task to run.
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
 * Takes care of process spawning/waiting and crinitTaskDB_t status updates.
 *
 * @param args  See ebcl_DispThrArgs_t.
 */
static void *EBCL_dispatchThreadFunc(void *args);
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

/**
 * Adds an action to a posix_spawn_file_actions_t instance as defined by an crinitIoRedir_t instance.
 *
 * The file action will either be a call to open() including an IO redirection for an crinitIoRedir_t which contains a
 * non-NULL crinitIoRedir_t::path, or a single redirection via dup2() if crinitIoRedir_t::oldfd is positive. If both cases
 * are true, the crinitIoRedir_t::path takes precedence and ebcl_IoRedir_r::oldfd is ignored.
 *
 * crinitIoRedir_t::oflags is respected for files and crinitIoRedir_t::mode is respected for files that are overwritten
 * or newly created.
 *
 * @param fileact  The posix_spawn_file_actions_t to be modified, must have been initialized.
 * @param ior      The crinitIoRedir_t defining the file action to be added.
 *
 * @return 0 on success, -1 otherwise.
 */
static int EBCL_posixSpawnAddIOFileAction(posix_spawn_file_actions_t *fileact, const crinitIoRedir_t *ior);

/**
 * Ensures the given path is a FIFO special file (named pipe).
 *
 * If the file exists it will be checked wie stat() and the function will return an error if the file is anything else
 * than a FIFO. If the file does not yet exist, it will be created with the given mode. Existing FIFOs will be left as
 * is.
 *
 * @param path  The path where the FIFO shall exist.
 * @param mode  The mode of a FIFO if it is newly created.
 *
 * @return  0 on success, -1 otherwise.
 */
static int EBCL_ensureFifo(const char *path, mode_t mode);

int crinitProcDispatchSpawnFunc(crinitTaskDB_t *ctx, const crinitTask_t *t) {
    pthread_t dispatchThread;
    pthread_attr_t dispatchThreadAttr;
    ebcl_DispThrArgs_t *threadArgs = malloc(sizeof(ebcl_DispThrArgs_t));
    if (threadArgs == NULL) {
        crinitErrnoPrint("Could not allocate memory for thread arguments. Meant to create thread for task \'%s\'.",
                        t->name);
        return -1;
    }
    threadArgs->ctx = ctx;
    threadArgs->t = t;

    if ((errno = pthread_attr_init(&dispatchThreadAttr)) != 0) {
        crinitErrnoPrint("Could not initialize pthread attributes. Meant to create thread for task \'%s\'.", t->name);
        free(threadArgs);
        return -1;
    }

    if ((errno = pthread_attr_setdetachstate(&dispatchThreadAttr, PTHREAD_CREATE_DETACHED)) != 0) {
        crinitErrnoPrint("Could not set PTHREAD_CREATE_DETACHED attribute. Meant to create thread for task \'%s\'.",
                        t->name);
        goto fail;
    }

    if ((errno = pthread_attr_setstacksize(&dispatchThreadAttr, CRINIT_PROC_DISPATCH_THREAD_STACK_SIZE)) != 0) {
        crinitErrnoPrint("Could not set pthread stack size to %d. Meant to create thread for task \'%s\'.",
                        CRINIT_PROC_DISPATCH_THREAD_STACK_SIZE, t->name);
        goto fail;
    }

    crinitDbgInfoPrint("Creating new thread for Task \'%s\'.", t->name);
    if ((errno = pthread_create(&dispatchThread, &dispatchThreadAttr, EBCL_dispatchThreadFunc, threadArgs)) != 0) {
        crinitErrnoPrint("Could not create pthread for task \'%s\'.", t->name);
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
    crinitTaskDB_t *ctx = a->ctx;
    const crinitTask_t *t = a->t;
    crinitTask_t *tCopy = NULL;
    pid_t threadId = crinitGettid();
    pid_t pid = -1;

    crinitDbgInfoPrint("(TID: %d) New thread started.", threadId);

    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("(TID: %d) Could not queue up for mutex lock.", threadId);
        goto threadExit;
    }

    if (crinitTaskDup(&tCopy, t) == -1) {
        crinitErrPrint("(TID: %d) Could not get duplicate of Task to spawn.", threadId);
        pthread_mutex_unlock(&ctx->lock);
        goto threadExit;
    }

    pthread_mutex_unlock(&ctx->lock);

    if (crinitEnvSetSet(&tCopy->taskEnv, CRINIT_ENV_NOTIFY_NAME, tCopy->name) == -1) {
        crinitErrPrint("Could not set notification environment variable for task \'%s\'", tCopy->name);
        goto threadExit;
    }

    crinitDbgInfoPrint("(TID: %d) Will spawn Task \'%s\'.", threadId, tCopy->name);

    for (size_t i = 0; i < tCopy->cmdsSize; i++) {
        posix_spawn_file_actions_t fileact;
        errno = posix_spawn_file_actions_init(&fileact);
        if (errno != 0) {
            crinitErrnoPrint("(TID: %d) Could not initialize posix_spawn file actions for command %zu of Task '%s'",
                            threadId, i, tCopy->name);
            goto threadExit;
        }

        for (size_t j = 0; j < tCopy->redirsSize; j++) {
            // NOTE: We currently have a umask of 0022 which precludes us from creating files with 0666 permissions.
            //       We may want to make that configurable in the future.

            if (tCopy->redirs[j].fifo && EBCL_ensureFifo(tCopy->redirs[j].path, tCopy->redirs[j].mode) == -1) {
                crinitErrPrint(
                    "(TID: %d) Unexpected result while ensuring '%s' is a FIFO special file for command %zu of Task "
                    "'%s'",
                    threadId, tCopy->redirs[j].path, i, tCopy->name);
                goto threadExit;
            }

            if (EBCL_posixSpawnAddIOFileAction(&fileact, &(tCopy->redirs[j])) == -1) {
                crinitErrPrint("(TID: %d) Could not add IO file action to posix_spawn for command %zu of Task '%s'",
                              threadId, i, tCopy->name);
                goto threadExit;
            }
        }

        errno = posix_spawn(&pid, tCopy->cmds[i].argv[0], &fileact, NULL, tCopy->cmds[i].argv, tCopy->taskEnv.envp);
        if (errno != 0 || pid == -1) {
            crinitErrnoPrint("(TID: %d) Could not spawn new process for command %zu of Task \'%s\'", threadId, i,
                            tCopy->name);
            posix_spawn_file_actions_destroy(&fileact);
            goto threadExit;
        }
        posix_spawn_file_actions_destroy(&fileact);
        crinitInfoPrint("(TID: %d) Started new process %d for command %zu of Task \'%s\' (\'%s\').", threadId, pid, i,
                       tCopy->name, tCopy->cmds[i].argv[0]);

        if (crinitTaskDBSetTaskPID(ctx, pid, tCopy->name) == -1) {
            crinitErrPrint("(TID: %d) Could not set PID of Task \'%s\' to %d.", threadId, tCopy->name, pid);
            goto threadExit;
        }

        if (i == 0) {
            if (crinitTaskDBSetTaskState(ctx, CRINIT_TASK_STATE_RUNNING, tCopy->name) == -1) {
                crinitErrPrint("(TID: %d) Could not set state of Task \'%s\' to running.", threadId, tCopy->name);
                goto threadExit;
            }
            char depEvent[sizeof(CRINIT_TASK_EVENT_RUNNING)] = CRINIT_TASK_EVENT_RUNNING;
            crinitTaskDep_t spawnDep = {tCopy->name, depEvent};
            if (crinitTaskDBFulfillDep(ctx, &spawnDep) == -1) {
                crinitErrPrint("(TID: %d) Could not fulfill dependency %s:%s.", threadId, spawnDep.name, spawnDep.event);
                goto threadExit;
            }
            crinitDbgInfoPrint("(TID: %d) Dependency \'%s:%s\' fulfilled.", threadId, spawnDep.name, spawnDep.event);

            if (crinitTaskDBProvideFeature(ctx, tCopy, CRINIT_TASK_STATE_RUNNING) == -1) {
                crinitErrPrint("(TID: %d) Could not fulfill provided features of spawned task \'%s\'.", threadId,
                              tCopy->name);
            }
            crinitDbgInfoPrint("(TID: %d) Features of spawned task \'%s\' fulfilled.", threadId, tCopy->name);
        }

        int wret;
        siginfo_t status;
        // Check if process has exited, but leave zombie.
        do {
            wret = waitid(P_PID, pid, &status, WEXITED | WNOWAIT);
        } while (wret != 0 && errno == EINTR);

        if (wret != 0 || status.si_code != CLD_EXITED || status.si_status != 0) {
            // There was some error, either Crinit-internal or the task returned an error code or the task was
            // killed
            if (errno) {
                crinitErrnoPrint("(TID: %d) Failed to wait for Task \'%s\' (PID %d).", threadId, tCopy->name, pid);
                goto threadExit;
            } else if (status.si_code == CLD_EXITED) {
                crinitInfoPrint("(TID: %d) Task \'%s\' (PID %d) returned error code %d.", threadId, tCopy->name, pid,
                               status.si_status);
            } else {
                crinitInfoPrint("(TID: %d) Task \'%s\' (PID %d) failed.", threadId, tCopy->name, pid);
            }

            if (crinitTaskDBSetTaskState(ctx, CRINIT_TASK_STATE_FAILED, tCopy->name) == -1) {
                crinitErrPrint("(TID: %d) Could not set state of Task \'%s\' to failed.", threadId, tCopy->name);
                goto threadExit;
            }
            if (crinitTaskDBSetTaskPID(ctx, -1, tCopy->name) == -1) {
                crinitErrPrint("(TID: %d) Could not reset PID of failed Task \'%s\' to -1.", threadId, tCopy->name);
                goto threadExit;
            }
            // Reap zombie of failed command.
            if (EBCL_reapPid(pid) == -1) {
                crinitErrnoPrint("(TID: %d) Could not reap zombie for task \'%s\'.", threadId, tCopy->name);
            }

            char depEvent[sizeof(CRINIT_TASK_EVENT_FAILED)] = CRINIT_TASK_EVENT_FAILED;
            crinitTaskDep_t failDep = {tCopy->name, depEvent};
            if (crinitTaskDBFulfillDep(ctx, &failDep) == -1) {
                crinitErrPrint("(TID: %d) Could not fulfill dependency %s:%s.", threadId, failDep.name, failDep.event);
            }
            crinitDbgInfoPrint("(TID: %d) Dependency \'%s:%s\' fulfilled.", threadId, failDep.name, failDep.event);

            if (crinitTaskDBProvideFeature(ctx, tCopy, CRINIT_TASK_STATE_FAILED) == -1) {
                crinitErrPrint("(TID: %d) Could not fulfill provided features of failed task \'%s\'.", threadId,
                              tCopy->name);
            }
            crinitDbgInfoPrint("(TID: %d) Features of failed task \'%s\' fulfilled.", threadId, tCopy->name);

            goto threadExit;
        }

        // command of task has returned successfully
        if (crinitTaskDBSetTaskPID(ctx, -1, tCopy->name) == -1) {
            crinitErrPrint("(TID: %d) Could not reset PID of Task \'%s\' to -1.", threadId, tCopy->name);
            goto threadExit;
        }
        // Reap zombie of successful command.
        if (EBCL_reapPid(pid) == -1) {
            crinitErrnoPrint("(TID: %d) Could not reap zombie for task \'%s\'.", threadId, tCopy->name);
        }
    }

    // chain of commands is done successfully
    crinitInfoPrint("(TID: %d) Task \'%s\' done.", threadId, tCopy->name);
    if (crinitTaskDBSetTaskState(ctx, CRINIT_TASK_STATE_DONE, tCopy->name) == -1) {
        crinitErrPrint("(TID: %d) Could not set state of Task \'%s\' to done.", threadId, tCopy->name);
        goto threadExit;
    }
    char depEvent[sizeof(CRINIT_TASK_EVENT_DONE)] = CRINIT_TASK_EVENT_DONE;
    crinitTaskDep_t doneDep = {tCopy->name, depEvent};
    if (crinitTaskDBFulfillDep(ctx, &doneDep) == -1) {
        crinitErrPrint("(TID: %d) Could not fulfill dependency %s:%s.", threadId, doneDep.name, doneDep.event);
    }
    crinitDbgInfoPrint("(TID: %d) Dependency \'%s:%s\' fulfilled.", threadId, doneDep.name, doneDep.event);

    if (crinitTaskDBProvideFeature(ctx, tCopy, CRINIT_TASK_STATE_DONE) == -1) {
        crinitErrPrint("(TID: %d) Could not fulfill provided features of finished task \'%s\'.", threadId, tCopy->name);
    }
    crinitDbgInfoPrint("(TID: %d) Features of finished task \'%s\' fulfilled.", threadId, tCopy->name);

threadExit:
    crinitFreeTask(tCopy);
    free(args);
    return NULL;
}

int crinitSetInhibitWait(bool inh) {
    int ret = 0;
    errno = pthread_mutex_lock(&EBCL_waitInhibitLock);
    if (errno != 0) {
        crinitErrnoPrint("Could not lock on mutex.");
        return -1;
    }
    EBCL_waitInhibit = inh;
    if (!inh) {
        errno = pthread_cond_broadcast(&EBCL_waitInhibitCond);
        if (errno != 0) {
            ret = -1;
            crinitErrnoPrint("Could not broadcast on condition variable.");
        }
    }
    if (pthread_mutex_unlock(&EBCL_waitInhibitLock)) {
        ret = -1;
        crinitErrnoPrint("Could not unlock mutex.");
    }
    return ret;
}

static int EBCL_blockOnWaitInhibit(void) {
    errno = pthread_mutex_lock(&EBCL_waitInhibitLock);
    if (errno != 0) {
        crinitErrnoPrint("Could not lock on mutex.");
        return -1;
    }
    while (EBCL_waitInhibit) {
        errno = pthread_cond_wait(&EBCL_waitInhibitCond, &EBCL_waitInhibitLock);
        if (errno != 0) {
            crinitErrnoPrint("Could not wait on condition variable.");
            return -1;
        }
    }
    if (pthread_mutex_unlock(&EBCL_waitInhibitLock)) {
        crinitErrnoPrint("Could not unlock mutex.");
        return -1;
    }
    return 0;
}

static int EBCL_reapPid(pid_t pid) {
    if (EBCL_blockOnWaitInhibit() == -1) {
        crinitErrPrint("Could not block on wait inhibition condition.");
        return -1;
    }
    do {
        waitpid(pid, NULL, 0);
    } while (errno == EINTR);
    if (errno != 0) {
        crinitErrnoPrint("Could not reap zombie for PID \'%d\'.", pid);
        return -1;
    }
    return 0;
}

static int EBCL_posixSpawnAddIOFileAction(posix_spawn_file_actions_t *fileact, const crinitIoRedir_t *ior) {
    if (fileact == NULL || ior == NULL) {
        crinitErrPrint("Input parameters must not be NULL.");
        return -1;
    }
    errno = 0;
    int streamFd = ior->newFd;
    if (streamFd != STDOUT_FILENO && streamFd != STDERR_FILENO && streamFd != STDIN_FILENO) {
        crinitErrPrint("Given IO stream must be either stdout, stderr, or stdin.");
        return -1;
    }
    if (ior->path != NULL) {
        errno = posix_spawn_file_actions_addopen(fileact, streamFd, ior->path, ior->oflags, ior->mode);
        if (errno != 0) {
            crinitErrnoPrint("Could not add file redirection to posix spawn.");
            return -1;
        }
        return 0;
    }

    if (ior->oldFd == -1) {
        crinitErrPrint("An IO redirection must specify either a path or a stream file descriptor as a target.");
        return -1;
    }

    errno = posix_spawn_file_actions_adddup2(fileact, ior->oldFd, ior->newFd);
    if (errno != 0) {
        crinitErrnoPrint("Could not add stream fd redirection to posix spawn.");
        return -1;
    }
    return 0;
}

static int EBCL_ensureFifo(const char *path, mode_t mode) {
    if (path == NULL) {
        crinitErrPrint("Path to the FIFO must not be NULL.");
        return -1;
    }

    // Try to create the FIFO and handle errors accordingly.
    if (mkfifo(path, mode) == -1) {
        if (errno == EEXIST) {
            // There already is something there, check what it is.
            struct stat stbuf;
            if (stat(path, &stbuf) == -1) {
                crinitErrnoPrint(
                    "There is already an existing file at '%s' but I can not stat() it to make sure it is a FIFO.",
                    path);
                return -1;
            } else if (!S_ISFIFO(stbuf.st_mode)) {
                crinitErrPrint("The given file '%s' already exists but is not a FIFO.", path);
                return -1;
            } else {
                // If we're here, that means there already is a FIFO and we're good.
                return 0;
            }
        }
        crinitErrnoPrint("Could not create FIFO special file at '%s'.", path);
        return -1;
    }
    return 0;
}
