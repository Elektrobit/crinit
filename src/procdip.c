// SPDX-License-Identifier: MIT
/**
 * @file procdip.c
 * @brief Implementation of the Process Dispatcher.
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
#include "lexers.h"
#include "logio.h"

#ifndef SYS_gettid
#error "SYS_gettid unavailable on this system"
#endif

/** Macro wrapper for the gettid syscall in case glibc is not new enough to contain one itself **/
#define crinitGettid() ((pid_t)syscall(SYS_gettid))

/** Struct wrapper for arguments to dispatchThreadFunc **/
typedef struct crinitDispThrArgs_t {
    crinitTaskDB_t *ctx;    ///< The TaskDB context to update on task state changes.
    const crinitTask_t *t;  ///< The task to run.
    crinitDispatchThreadMode_t mode;    ///< Select between start and stop commands
} crinitDispThrArgs_t;

/** Mutex to guard #crinitWaitInhibit **/
static pthread_mutex_t crinitWaitInhibitLock = PTHREAD_MUTEX_INITIALIZER;
/** Condition variable to signal threads waiting for #crinitWaitInhibit to become `false`. **/
static pthread_cond_t crinitWaitInhibitCond = PTHREAD_COND_INITIALIZER;
/** If true, all terminated child processes will be kept around as zombies (see crinitBlockOnWaitInhibit()). **/
static bool crinitWaitInhibit = false;

/**
 * Function to be started as a pthread from crinitProcDispatchThread().
 *
 * Takes care of process spawning/waiting and crinitTaskDB_t status updates.
 *
 * @param args  See crinitDispThrArgs_t.
 */
static void *crinitDispatchThreadFunc(void *args);
/**
 * Block calling thread until #crinitWaitInhibit becomes false.
 *
 * Called by dispatcher threads to delay reaping of zombie processes if needed.
 *
 * @return 0 on success, -1 on error
 */
static int crinitBlockOnWaitInhibit(void);
/**
 * Reap a zombie process.
 *
 * Will call blockOnWaitInhibit() internally.
 *
 * @param pid  The PID of the process to wait for.
 *
 * @return 0 on success, -1 on error
 */
static int crinitReapPid(pid_t pid);

/**
 * Adds an action to a posix_spawn_file_actions_t instance as defined by an crinitIoRedir_t instance.
 *
 * The file action will either be a call to open() including an IO redirection for an crinitIoRedir_t which contains a
 * non-NULL crinitIoRedir_t::path, or a single redirection via dup2() if crinitIoRedir_t::oldfd is positive. If both
 * cases are true, the crinitIoRedir_t::path takes precedence and crinitIoRedir_r::oldfd is ignored.
 *
 * crinitIoRedir_t::oflags is respected for files and crinitIoRedir_t::mode is respected for files that are overwritten
 * or newly created.
 *
 * @param fileact  The posix_spawn_file_actions_t to be modified, must have been initialized.
 * @param ior      The crinitIoRedir_t defining the file action to be added.
 *
 * @return 0 on success, -1 otherwise.
 */
static int crinitPosixSpawnAddIOFileAction(posix_spawn_file_actions_t *fileact, const crinitIoRedir_t *ior);

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
static int crinitEnsureFifo(const char *path, mode_t mode);

int crinitProcDispatchSpawnFunc(crinitTaskDB_t *ctx, const crinitTask_t *t, crinitDispatchThreadMode_t mode) {
    pthread_t dispatchThread;
    pthread_attr_t dispatchThreadAttr;
    crinitDispThrArgs_t *threadArgs = malloc(sizeof(crinitDispThrArgs_t));
    if (threadArgs == NULL) {
        crinitErrnoPrint("Could not allocate memory for thread arguments. Meant to create thread for task \'%s\'.",
                         t->name);
        return -1;
    }
    threadArgs->ctx = ctx;
    threadArgs->t = t;
    threadArgs->mode = mode;

    if ((errno = pthread_attr_init(&dispatchThreadAttr)) != 0) {
        crinitErrnoPrint("Could not initialize pthread attributes. Meant to create thread for task \'%s\'.", t->name);
        goto fail;
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
    if ((errno = pthread_create(&dispatchThread, &dispatchThreadAttr, crinitDispatchThreadFunc, threadArgs)) != 0) {
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

int crinitSpawnSingleCommand(const char* cmd, char* const argv[], char* const envp[], posix_spawn_file_actions_t *fileact, const char* name, size_t cmdIdx, pid_t threadId, pid_t *pid)
{
    errno = posix_spawn(pid, cmd, fileact, NULL, argv, envp);
    if (errno != 0 || *pid == -1) {
        crinitErrnoPrint("(TID: %d) Could not spawn new process for command %zu of Task \'%s\'", threadId, cmdIdx,
                         name);
        return -1;
    }
    return 0;
}

int crinitPrepareIoRedirectionsForSpawn(size_t cmdIdx, crinitIoRedir_t * redirs, size_t redirsSize, pid_t threadId, const char *name, posix_spawn_file_actions_t *fileact)
{
    for (size_t j = 0; j < redirsSize; j++) {
        // NOTE: We currently have a umask of 0022 which precludes us from creating files with 0666 permissions.
        //       We may want to make that configurable in the future.

        if (redirs[j].fifo && crinitEnsureFifo(redirs[j].path, redirs[j].mode) == -1) {
            crinitErrPrint(
                "(TID: %d) Unexpected result while ensuring '%s' is a FIFO special file for command %zu of Task "
                "'%s'",
                threadId, redirs[j].path, cmdIdx, name);
            return -1;
        }

        if (crinitPosixSpawnAddIOFileAction(fileact, &(redirs[j])) == -1) {
            crinitErrPrint("(TID: %d) Could not add IO file action to posix_spawn for command %zu of Task '%s'",
                           threadId, cmdIdx, name);
            return -1;
        }
    }
    return 0;
}

int crinitHandleCommands(crinitTaskDB_t *ctx, pid_t threadId, char* name, crinitTaskCmd_t *cmds, size_t cmdsSize, crinitTask_t *tCopy, pid_t *pid)
{
    for (size_t i = 0; i < cmdsSize; i++) {
        posix_spawn_file_actions_t fileact;
        errno = posix_spawn_file_actions_init(&fileact);
        if (errno != 0) {
            crinitErrnoPrint("(TID: %d) Could not initialize posix_spawn file actions for command %zu of Task '%s'",
                             threadId, i, name);
            posix_spawn_file_actions_destroy(&fileact);
            return -1;
        }

        if (crinitPrepareIoRedirectionsForSpawn(i, tCopy->redirs, tCopy->redirsSize, threadId, tCopy->name, &fileact) == -1) {
            posix_spawn_file_actions_destroy(&fileact);
            return -1;
        }

        if (crinitSpawnSingleCommand(cmds[i].argv[0], cmds[i].argv, tCopy->taskEnv.envp, &fileact, name, i, threadId, pid) == -1) {
            posix_spawn_file_actions_destroy(&fileact);
            return -1;
        }
        posix_spawn_file_actions_destroy(&fileact);

        crinitInfoPrint("(TID: %d) Started new process %d for command %zu of Task \'%s\' (\'%s\').", threadId, *pid, i,
                        name, cmds[i].argv[0]);

        if (crinitTaskDBSetTaskPID(ctx, *pid, name) == -1) {
            crinitErrPrint("(TID: %d) Could not set PID of Task \'%s\' to %d.", threadId, name, *pid);
            return -1;
        }

        if (i == 0) {
            if (crinitTaskDBSetTaskState(ctx, CRINIT_TASK_STATE_RUNNING, name) == -1) {
                crinitErrPrint("(TID: %d) Could not set state of Task \'%s\' to running.", threadId, name);
                return -1;
            }
            crinitTaskDep_t spawnDep = {name, CRINIT_TASK_EVENT_RUNNING};
            if (crinitTaskDBFulfillDep(ctx, &spawnDep, NULL) == -1) {
                crinitErrPrint("(TID: %d) Could not fulfill dependency %s:%s.", threadId, spawnDep.name,
                               spawnDep.event);
                return -1;
            }
            crinitDbgInfoPrint("(TID: %d) Dependency \'%s:%s\' fulfilled.", threadId, spawnDep.name, spawnDep.event);

            if (crinitTaskDBProvideFeature(ctx, tCopy, CRINIT_TASK_STATE_RUNNING) == -1) {
                crinitErrPrint("(TID: %d) Could not fulfill provided features of spawned task \'%s\'.", threadId,
                               name);
            }
            crinitDbgInfoPrint("(TID: %d) Features of spawned task \'%s\' fulfilled.", threadId, name);
        }

        int wret;
        siginfo_t status;
        // Check if process has exited, but leave zombie.
        do {
            wret = waitid(P_PID, *pid, &status, WEXITED | WNOWAIT);
        } while (wret != 0 && errno == EINTR);

        if (wret != 0 || status.si_code != CLD_EXITED || status.si_status != 0) {
            // There was some error, either Crinit-internal or the task returned an error code or the task was killed.
            if (errno) {
                crinitErrnoPrint("(TID: %d) Failed to wait for Task \'%s\' (PID %d).", threadId, name, *pid);
                return -1;
            } else if (status.si_code == CLD_EXITED) {
                crinitInfoPrint("(TID: %d) Task \'%s\' (PID %d) returned error code %d.", threadId, name, *pid,
                                status.si_status);
            } else {
                crinitInfoPrint("(TID: %d) Task \'%s\' (PID %d) failed.", threadId, name, *pid);
            }
            return -1;
        }

        // command of task has returned successfully
        if (crinitTaskDBSetTaskPID(ctx, -1, name) == -1) {
            crinitErrPrint("(TID: %d) Could not reset PID of Task \'%s\' to -1.", threadId, name);
        }
        // Reap zombie of successful command.
        if (crinitReapPid(*pid) == -1) {
            crinitErrnoPrint("(TID: %d) Could not reap zombie for task \'%s\'.", threadId, name);
        }
    }

    return 0;
}

int crinitExpandPIDVariablesInSingleCommand(char *input, const pid_t pid, char **result) {
    crinitTokenType_t tt;
    char *substKey = NULL;
    char *substVal = NULL;
    const char *src = input;
    const char *s = input;
    const char *mbegin = NULL;
    const char *mend = NULL;

    const int substValLen = snprintf(NULL, 0, "%d", pid);
    substVal = (char *) calloc(substValLen + 1, sizeof(char));
    if (!substVal) {
        crinitErrnoPrint("Couldn't write PID into temporary string.\n");
        return -1;
    }
    snprintf(substVal, substValLen + 1, "%d", pid);
    if (*result != NULL) {
        crinitErrnoPrint("Expecting result pointer to be NULL.\n");
        return -1;
    }

    do {
        tt = crinitEnvVarInnerLex(&s, &mbegin, &mend);
        switch (tt) {
            case CRINIT_TK_ERR:
                crinitErrPrint("Error while parsing string at '%.*s'\n", (int)(mend - mbegin), mbegin);
                break;
            case CRINIT_TK_VAR:
                substKey = strndup(mbegin, (size_t)(mend - mbegin));
                if (substKey == NULL) {
                    crinitErrnoPrint("Could not duplicate key of environment variable to expand.");
                    tt = CRINIT_TK_ERR;
                    break;
                }
                if (strcmp(substKey, "TASK_PID") == 0) {
                    free(substKey);
                    char *tmp = NULL;
                    tmp = calloc((int) (mend - src) + substValLen + 1, sizeof(char));
                    if (!tmp) {
                        crinitErrPrint("Error allocating memory for result string.\n");
                        tt = CRINIT_TK_ERR;
                        break;
                    }
                    sprintf(tmp, "%.*s%s", (int) (mbegin - 2 - src), src, substVal);        // Substitute 2 to get rid of '${' directly before the variable name.
                    if (*result) {
                        char *tmp2 = NULL;
                        tmp2 = (char *) calloc(strlen(*result) + strlen(tmp) + 1, sizeof(char));
                        if (!tmp2) {
                            crinitErrPrint("Error allocating memory for result string.\n");
                            tt = CRINIT_TK_ERR;
                            break;
                        }
                        strcat(tmp2, *result);
                        strcat(tmp2, tmp);
                        free(*result);
                        *result = tmp2;
                    }
                    else {
                        free(*result);
                        *result = tmp;
                    }
                    src = mend + 1;
                }
                else {
                    free(substKey);
                }
                break;
            case CRINIT_TK_END:
            case CRINIT_TK_CPY:
                // Intentionally do nothing.
                break;
            case CRINIT_TK_ESC:
            case CRINIT_TK_ESCX:
            case CRINIT_TK_WSPC:
            case CRINIT_TK_ENVKEY:
            case CRINIT_TK_ENVVAL:
            case CRINIT_TK_UQSTR:
            case CRINIT_TK_DQSTR:
            default:
                crinitErrPrint("Parser error at '%.*s'\n", (int)(mend - mbegin), mbegin);
                tt = CRINIT_TK_ERR;
                break;
        }
    } while (tt != CRINIT_TK_END && tt != CRINIT_TK_ERR);

    if (tt == CRINIT_TK_END && *result) {
        char *tmp = NULL;
        tmp = (char *) calloc(strlen(*result) + strlen(src) + 1, sizeof(char));
        if (!tmp) {
            crinitErrPrint("Error allocating memory for result string.\n");
            tt = CRINIT_TK_ERR;
            return -1;
        }
        strcat(tmp, *result);
        strcat(tmp, src);
        free(*result);
        *result = tmp;
        return 1;
    }
    else if (tt == CRINIT_TK_ERR) {
        free(*result);
        return -1;
    }

    return 0;
}

/**
 * @brief Expands variable ${TASK_PID} in a command with the task's PID.
 *
 * Please note that this makes only sense for a STOP_COMMAND.
 * Currently this is the only variable that can be expanded here.
 *
 * @param commands Pointer to task command structure
 * @param cmdsSize Number of elements in commands
 * @param pid PID of task
 */
void crinitExpandPIDVariablesInCommands(crinitTaskCmd_t *commands, size_t cmdsSize, const pid_t pid) {
    for (size_t i = 0; i < cmdsSize; i++) {
        if (commands[i].argc > 0) {
            char *origArgvBackbufEnd = strchr(commands[i].argv[commands[i].argc - 1], '\0');
            size_t argvBackbufLen = origArgvBackbufEnd - commands[i].argv[0] + 1;
            argvBackbufLen *= 2;
            size_t bytesUsed = 0;

            char *argvBackbuf = malloc(argvBackbufLen);
            if (argvBackbuf == NULL) {
                crinitErrnoPrint("Could not allocate memory for argv STOP_COMMAND variable expansion.\n");
                return;
            }

            char *oldArgvBuf = commands[i].argv[0];
            size_t *offsets = calloc(sizeof(size_t), commands[i].argc);
            for (int j = 0; j < commands[i].argc; j++) {
                char *result = NULL;
                if (crinitExpandPIDVariablesInSingleCommand(commands[i].argv[j], pid, &result) == -1) {
                    crinitErrnoPrint("Could not allocate memory for argv STOP_COMMAND variable expansion.\n");
                    free(argvBackbuf);
                    free(offsets);
                    return;
                }
                else {
                    char *tmpResult = commands[i].argv[j];
                    if (result) {
                        tmpResult = result;
                    }
                    const size_t resultLen = strlen(tmpResult) + 1;
                    if (bytesUsed + resultLen > argvBackbufLen) {
                        char *tmp = realloc(argvBackbuf, bytesUsed + resultLen);
                        if (tmp == NULL) {
                            free(result);
                            free(argvBackbuf);
                            free(offsets);
                            return;
                        }
                        argvBackbuf = tmp;
                        argvBackbufLen = bytesUsed + resultLen;
                    }
                    memcpy(argvBackbuf + bytesUsed, tmpResult, resultLen);
                    offsets[j] = bytesUsed;
                    bytesUsed += resultLen;
                    free(result);
                }
            }

            if (argvBackbufLen > bytesUsed) {
                argvBackbuf = realloc(argvBackbuf, bytesUsed);
            }
            free(oldArgvBuf);
            for (int j = 0; j < commands[i].argc; j++) {
                commands[i].argv[j] = argvBackbuf + offsets[j];
            }
            free(offsets);
        }
    }
}

static void *crinitDispatchThreadFunc(void *args) {
    crinitDispThrArgs_t *a = (crinitDispThrArgs_t *)args;
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
        goto threadExitFail;
    }

    crinitDbgInfoPrint("(TID: %d) Will spawn Task \'%s\'.", threadId, tCopy->name);

    crinitTaskCmd_t *cmds = NULL;
    size_t cmdsSize = 0;
    switch (a->mode) {
        case CRINIT_DISPATCH_THREAD_MODE_START:
            cmds = tCopy->cmds;
            cmdsSize = tCopy->cmdsSize;
            break;
        case CRINIT_DISPATCH_THREAD_MODE_STOP:
            cmds = tCopy->stopCmds;
            cmdsSize = tCopy->stopCmdsSize;
            crinitExpandPIDVariablesInCommands(cmds, cmdsSize, tCopy->pid);
            break;
        default:
            crinitErrPrint("Invalid mode for dispatch thread work mode received");
            goto threadExitFail;
    }

    //TODO: Hier Variablenersetzung vornehmen. Eventuell nur im CRINIT_DISPATCH_THREAD_MODE_STOP Mode?
    // TASK_PID macht ja nur beim STOP sinn.

    if (crinitHandleCommands(ctx, threadId, tCopy->name, cmds, cmdsSize, tCopy, &pid) != 0) {
        goto threadExitFail;
    }

    // chain of commands is done successfully
    crinitInfoPrint("(TID: %d) Task \'%s\' done.", threadId, tCopy->name);
    if (crinitTaskDBSetTaskState(ctx, CRINIT_TASK_STATE_DONE, tCopy->name) == -1) {
        crinitErrPrint("(TID: %d) Could not set state of Task \'%s\' to done.", threadId, tCopy->name);
    }
    crinitTaskDep_t doneDep = {tCopy->name, CRINIT_TASK_EVENT_DONE};
    if (crinitTaskDBFulfillDep(ctx, &doneDep, NULL) == -1) {
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

threadExitFail:
    if (crinitTaskDBSetTaskState(ctx, CRINIT_TASK_STATE_FAILED, tCopy->name) == -1) {
        crinitErrPrint("(TID: %d) Could not set state of Task \'%s\' to failed.", threadId, tCopy->name);
    }
    if (crinitTaskDBSetTaskPID(ctx, -1, tCopy->name) == -1) {
        crinitErrPrint("(TID: %d) Could not reset PID of failed Task \'%s\' to -1.", threadId, tCopy->name);
    }
    // Reap zombie of failed command (if it was actually spawned).
    if (pid > 0 && crinitReapPid(pid) == -1) {
        crinitErrPrint("(TID: %d) Could not reap zombie for task \'%s\'.", threadId, tCopy->name);
    }

    crinitTaskDep_t failDep = {tCopy->name, CRINIT_TASK_EVENT_FAILED};
    if (crinitTaskDBFulfillDep(ctx, &failDep, NULL) == -1) {
        crinitErrPrint("(TID: %d) Could not fulfill dependency %s:%s.", threadId, failDep.name, failDep.event);
    } else {
        crinitDbgInfoPrint("(TID: %d) Dependency \'%s:%s\' fulfilled.", threadId, failDep.name, failDep.event);
    }

    if (crinitTaskDBProvideFeature(ctx, tCopy, CRINIT_TASK_STATE_FAILED) == -1) {
        crinitErrPrint("(TID: %d) Could not fulfill provided features of failed task \'%s\'.", threadId, tCopy->name);
    } else {
        crinitDbgInfoPrint("(TID: %d) Features of failed task \'%s\' fulfilled.", threadId, tCopy->name);
    }

    crinitFreeTask(tCopy);
    free(args);
    return NULL;
}

int crinitSetInhibitWait(bool inh) {
    int ret = 0;
    errno = pthread_mutex_lock(&crinitWaitInhibitLock);
    if (errno != 0) {
        crinitErrnoPrint("Could not lock on mutex.");
        return -1;
    }
    crinitWaitInhibit = inh;
    if (!inh) {
        errno = pthread_cond_broadcast(&crinitWaitInhibitCond);
        if (errno != 0) {
            ret = -1;
            crinitErrnoPrint("Could not broadcast on condition variable.");
        }
    }
    if (pthread_mutex_unlock(&crinitWaitInhibitLock)) {
        ret = -1;
        crinitErrnoPrint("Could not unlock mutex.");
    }
    return ret;
}

static int crinitBlockOnWaitInhibit(void) {
    errno = pthread_mutex_lock(&crinitWaitInhibitLock);
    if (errno != 0) {
        crinitErrnoPrint("Could not lock on mutex.");
        return -1;
    }
    while (crinitWaitInhibit) {
        errno = pthread_cond_wait(&crinitWaitInhibitCond, &crinitWaitInhibitLock);
        if (errno != 0) {
            crinitErrnoPrint("Could not wait on condition variable.");
            return -1;
        }
    }
    if (pthread_mutex_unlock(&crinitWaitInhibitLock)) {
        crinitErrnoPrint("Could not unlock mutex.");
        return -1;
    }
    return 0;
}

static int crinitReapPid(pid_t pid) {
    if (crinitBlockOnWaitInhibit() == -1) {
        crinitErrPrint("Could not block on wait inhibition condition.");
        return -1;
    }
    do {
        waitpid(pid, NULL, 0);
    } while (errno == EINTR);
    if (errno == ECHILD) {
        return 0;  // If the PID does not exist it has either already been reaped or never existed. In either case,
                   // we're fine.
    } else if (errno != 0) {
        crinitErrnoPrint("Could not reap zombie for PID \'%d\'.", pid);
        return -1;
    }
    return 0;
}

static int crinitPosixSpawnAddIOFileAction(posix_spawn_file_actions_t *fileact, const crinitIoRedir_t *ior) {
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

static int crinitEnsureFifo(const char *path, mode_t mode) {
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
