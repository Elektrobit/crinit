// SPDX-License-Identifier: MIT
/**
 * @file taskdb.c
 * @brief Implementation of the central Task Database and related functions.
 */
#include "taskdb.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "elos-common.h"
#include "eloslog.h"
#include "globopt.h"
#include "logio.h"
#include "optfeat.h"

/**
 * Find index of a task in the crinitTaskDB_t::taskSet of an crinitTaskDB_t by name.
 *
 * @param task      Pointer pointer to return the task with.
 * @param taskName  The crinitTask_t::name to search for.
 * @param in        The crinitTaskDB_t_t to search in.
 *
 * @return 0 on success, -1 otherwise
 */
static int crinitFindTask(crinitTask_t **task, const char *taskName, const crinitTaskDB_t *in);
/**
 * Check if an crinitTask_t is considered ready to be started (startable).
 *
 * See crinitTaskDBSpawnReady() for further explanation.
 *
 * @param t The task to be checked for readiness.
 *
 * @return true if \a t is ready, false otherwise
 */
static bool crinitTaskIsReady(const crinitTask_t *t);

int crinitTaskDBInitWithSize(crinitTaskDB_t *ctx, int (*spawnFunc)(crinitTaskDB_t *ctx, const crinitTask_t *),
                             size_t initialSize) {
    crinitNullCheck(-1, ctx);

    if (initialSize < 1) {
        crinitErrPrint("Given initial size of task set in TaskDB must be at least 1.");
        return -1;
    }
    ctx->taskSetSize = 0;
    ctx->taskSetItems = 0;
    ctx->spawnFunc = NULL;
    ctx->spawnInhibit = true;
    ctx->taskSet = calloc(initialSize, sizeof(*ctx->taskSet));
    if (ctx->taskSet == NULL) {
        crinitErrnoPrint("Could not allocate memory for Task set of size %zu in TaskDB.", initialSize);
        return -1;
    }

    if ((errno = pthread_mutex_init(&ctx->lock, NULL)) != 0) {
        crinitErrnoPrint("Could not initialize mutex for TaskDB.");
        goto fail;
    }
    if ((errno = pthread_cond_init(&ctx->changed, NULL)) != 0) {
        crinitErrnoPrint("Could not initialize condition variable for TaskDB.");
        pthread_mutex_destroy(&ctx->lock);
        goto fail;
    }

    ctx->taskSetSize = initialSize;
    ctx->spawnFunc = spawnFunc;
    ctx->spawnInhibit = false;
    return 0;
fail:
    free(ctx->taskSet);
    ctx->taskSet = NULL;
    return -1;
}

int crinitTaskDBDestroy(crinitTaskDB_t *ctx) {
    crinitNullCheck(-1, ctx);

    ctx->spawnFunc = NULL;
    ctx->taskSetSize = 0;
    for (size_t i = 0; i < ctx->taskSetItems; i++) {
        crinitDestroyTask(&ctx->taskSet[i]);
    }
    ctx->taskSetItems = 0;

    free(ctx->taskSet);
    int err = 0;
    if ((err = pthread_mutex_destroy(&ctx->lock)) != 0) {
        errno = err;
        crinitErrnoPrint("Could not destroy mutex in TaskDB.");
        return -1;
    }
    return 0;
}

int crinitTaskDBInsert(crinitTaskDB_t *ctx, const crinitTask_t *t, bool overwrite) {
    crinitNullCheck(-1, ctx, t);

    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }

    crinitTask_t *pTask;
    if (crinitFindTask(&pTask, t->name, ctx) == 0) {
        if (overwrite) {
            crinitDestroyTask(pTask);
        } else {
            crinitErrPrint("Found task/include with name '%s' already in TaskDB but will not overwrite", t->name);
            goto fail;
        }
    }

    if (pTask == NULL) {
        if (ctx->taskSetItems == ctx->taskSetSize) {
            // We need to grow the backing array
            crinitTask_t *newSet = realloc(ctx->taskSet, ctx->taskSetSize * 2 * sizeof(crinitTask_t));
            if (newSet == NULL) {
                crinitErrnoPrint("Could not allocate additional memory for more task/include elements.");
                goto fail;
            }
            ctx->taskSet = newSet;
            ctx->taskSetSize *= 2;
        }

        pTask = &ctx->taskSet[ctx->taskSetItems++];
    }

    if (crinitTaskCopy(pTask, t) == -1) {
        crinitErrPrint("Could not copy new Task.");
        goto fail;
    }

    if (crinitElosLog(ELOS_SEVERITY_INFO, ELOS_MSG_CODE_FILE_OPENED, pTask->name) == -1) {
        crinitErrPrint(
            "Could not enqueue elos task creation event for '%s'. Will continue but logging may be impaired.",
            pTask->name);
    }

    crinitDbgInfoPrint("Run feature hooks for 'TASK_ADDED'.");
    if (crinitFeatureHook(NULL, CRINIT_HOOK_TASK_ADDED, pTask) == -1) {
        crinitErrPrint("Could not run activiation hook for feature \'TASK_ADDED\'.");
        goto fail;
    }

    pthread_cond_broadcast(&ctx->changed);
    pthread_mutex_unlock(&ctx->lock);
    return 0;
fail:
    pthread_mutex_unlock(&ctx->lock);
    return -1;
}

int crinitTaskDBSpawnReady(crinitTaskDB_t *ctx) {
    crinitNullCheck(-1, ctx);

    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }

    if (ctx->spawnInhibit) {
        pthread_mutex_unlock(&ctx->lock);
        return 0;
    }

    if (ctx->spawnFunc == NULL) {
        crinitErrPrint("Could not spawn ready tasks because spawn function pointer was set to NULL.");
        pthread_mutex_unlock(&ctx->lock);
        return -1;
    }

    crinitTask_t *pTask;
    crinitTaskDbForEach(ctx, pTask) {
        if (crinitTaskIsReady(pTask)) {
            crinitDbgInfoPrint("Task \'%s\' ready to spawn.", pTask->name);
            pTask->state = CRINIT_TASK_STATE_STARTING;

            if (ctx->spawnFunc(ctx, pTask) == -1) {
                crinitErrPrint("Could not spawn new thread for execution of task \'%s\'.", pTask->name);
                pTask->state &= ~CRINIT_TASK_STATE_STARTING;
                pthread_mutex_unlock(&ctx->lock);
                return -1;
            }
        }
    }

    pthread_mutex_unlock(&ctx->lock);
    return 0;
}

int crinitTaskDBSetSpawnInhibit(crinitTaskDB_t *ctx, bool inh) {
    crinitNullCheck(-1, ctx);

    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }

    if (inh != ctx->spawnInhibit) {
        ctx->spawnInhibit = inh;
        if (!inh) {
            pthread_cond_broadcast(&ctx->changed);
        }
    }
    pthread_mutex_unlock(&ctx->lock);
    return 0;
}

int crinitTaskDBSetTaskState(crinitTaskDB_t *ctx, crinitTaskState_t s, const char *taskName) {
    crinitNullCheck(-1, ctx, taskName);

    struct timespec timestamp = {0};
    // Check if we need to timestamp this state change.
    if (s & (CRINIT_TASK_STATE_RUNNING | CRINIT_TASK_STATE_DONE | CRINIT_TASK_STATE_FAILED)) {
        if (clock_gettime(CLOCK_MONOTONIC, &timestamp) == -1) {
            crinitErrnoPrint("Could not measure timestamp for task '%s'. Will set to 0 (undefined) and carry on.",
                             taskName);
        }
    }

    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }

    crinitTask_t *pTask;
    if (crinitFindTask(&pTask, taskName, ctx) == 0) {
        crinitElosSeverityE_t elosSeverity = ELOS_SEVERITY_INFO;
        crinitElosEventMessageCodeE_t elosMsgCode = ELOS_MSG_CODE_INFO_LOG;
        pTask->state = s;
        s &= ~CRINIT_TASK_STATE_NOTIFIED;  // Here we don't care if we got the state via notification or directly.
        switch (s) {
            case CRINIT_TASK_STATE_FAILED:
                pTask->failCount++;
                memcpy(&pTask->endTime, &timestamp, sizeof(pTask->endTime));
                elosSeverity = ELOS_SEVERITY_ERROR;
                elosMsgCode = ELOS_MSG_CODE_EXIT_FAILURE;
                break;
            case CRINIT_TASK_STATE_DONE:
                pTask->failCount = 0;
                memcpy(&pTask->endTime, &timestamp, sizeof(pTask->endTime));
                elosMsgCode = ELOS_MSG_CODE_PROCESS_EXITED;
                break;
            case CRINIT_TASK_STATE_RUNNING:
                memcpy(&pTask->startTime, &timestamp, sizeof(pTask->startTime));
                elosMsgCode = ELOS_MSG_CODE_PROCESS_CREATED;
                break;
            default:
                // do nothing
                break;
        }
        pthread_cond_broadcast(&ctx->changed);
        pthread_mutex_unlock(&ctx->lock);
        if (crinitElosLog(elosSeverity, elosMsgCode, taskName) == -1) {
            crinitErrPrint("Could not send task event to elos. Will continue but logging may be impaired.");
        }
        return 0;
    }

    pthread_mutex_unlock(&ctx->lock);
    crinitErrPrint("Could not set TaskState for Task \'%s\' as it does not exist in TaskDB.", taskName);
    return -1;
}

int crinitTaskDBGetTaskState(crinitTaskDB_t *ctx, crinitTaskState_t *s, const char *taskName) {
    crinitNullCheck(-1, ctx, taskName, s);

    *s = 0;
    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }

    crinitTask_t *pTask;
    if (crinitFindTask(&pTask, taskName, ctx) == 0) {
        *s = pTask->state;
        pthread_mutex_unlock(&ctx->lock);
        return 0;
    }
    pthread_mutex_unlock(&ctx->lock);
    crinitErrPrint("Could not get TaskState of Task \'%s\' as it does not exist in TaskDB.", taskName);
    return -1;
}

int crinitTaskDBSetTaskPID(crinitTaskDB_t *ctx, pid_t pid, const char *taskName) {
    crinitNullCheck(-1, ctx, taskName);

    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }

    crinitTask_t *pTask;
    if (crinitFindTask(&pTask, taskName, ctx) == 0) {
        pTask->pid = pid;
        pthread_mutex_unlock(&ctx->lock);
        return 0;
    }
    pthread_mutex_unlock(&ctx->lock);
    crinitErrPrint("Could not set TaskState for Task \'%s\' as it does not exist in TaskDB.", taskName);
    return -1;
}

int crinitTaskDBGetTaskPID(crinitTaskDB_t *ctx, pid_t *pid, const char *taskName) {
    crinitNullCheck(-1, ctx, taskName, pid);

    *pid = -1;
    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }

    crinitTask_t *pTask;
    if (crinitFindTask(&pTask, taskName, ctx) == 0) {
        *pid = pTask->pid;
        pthread_mutex_unlock(&ctx->lock);
        return 0;
    }
    pthread_mutex_unlock(&ctx->lock);
    crinitErrPrint("Could not get TaskState of Task \'%s\' as it does not exist in TaskDB.", taskName);
    return -1;
}

int crinitTaskDBGetTaskStateAndPID(crinitTaskDB_t *ctx, crinitTaskState_t *s, pid_t *pid, const char *taskName) {
    crinitNullCheck(-1, ctx, taskName, s, pid);

    *s = 0;
    *pid = 0;
    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }

    crinitTask_t *pTask;
    if (crinitFindTask(&pTask, taskName, ctx) == 0) {
        *s = pTask->state;
        *pid = pTask->pid;
        pthread_mutex_unlock(&ctx->lock);
        return 0;
    }
    pthread_mutex_unlock(&ctx->lock);
    crinitErrPrint("Could not get TaskState of Task \'%s\' as it does not exist in TaskDB.", taskName);
    return -1;
}

crinitTask_t *crinitTaskDBBorrowTask(crinitTaskDB_t *ctx, const char *taskName) {
    crinitNullCheck(NULL, ctx, taskName);

    if ((errno = pthread_mutex_lock(&ctx->lock)) == -1) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return NULL;
    }

    crinitTask_t *pTask;
    if (crinitFindTask(&pTask, taskName, ctx) == -1) {
        crinitErrPrint("Could not find task '%s' in TaskDB.", taskName);
        pthread_mutex_unlock(&ctx->lock);
        return NULL;
    }
    return pTask;
}

int crinitTaskDBRemit(crinitTaskDB_t *ctx) {
    crinitNullCheck(-1, ctx);

    // This *could* be called from a thread which does not actually own the mutex, so we need to check if
    // pthread_mutex_unlock() fails.
    errno = pthread_mutex_unlock(&ctx->lock);
    if (errno != 0) {
        crinitErrnoPrint("Could not unlock task database mutex.");
        return -1;
    }
    return 0;
}

int crinitTaskDBAddDepToTask(crinitTaskDB_t *ctx, const crinitTaskDep_t *dep, const char *taskName) {
    crinitNullCheck(-1, ctx, dep, taskName);

    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }

    crinitTask_t *pTask;
    crinitTaskDep_t *pDep;
    if (crinitFindTask(&pTask, taskName, ctx) == 0) {
        // Return immediately if dependency is already present
        crinitTaskForEachDep(pTask, pDep) {
            if (strcmp(pDep->name, dep->name) == 0 && strcmp(pDep->event, dep->event) == 0) {
                pthread_mutex_unlock(&ctx->lock);
                return 0;
            }
        }
        pTask->depsSize++;
        crinitTaskDep_t *pTempDeps = realloc(pTask->deps, pTask->depsSize * sizeof(crinitTaskDep_t));
        if (pTempDeps == NULL) {
            crinitErrnoPrint("Could not reallocate memory of dependency array for task \'%s\'.", taskName);
            pTask->depsSize--;
            pthread_mutex_unlock(&ctx->lock);
            return -1;
        }
        pTask->deps = pTempDeps;
        size_t lastIdx = pTask->depsSize - 1;
        size_t nameCopyLen = strlen(dep->name) + 1;
        size_t eventCopyLen = strlen(dep->event) + 1;
        pTask->deps[lastIdx].name = malloc(nameCopyLen + eventCopyLen);

        if (pTask->deps[lastIdx].name == NULL) {
            crinitErrnoPrint("Could not allocate memory for dependency backing string for task \'%s\'.", taskName);
            pTask->depsSize--;
            pthread_mutex_unlock(&ctx->lock);
            return -1;
        }
        pTask->deps[lastIdx].event = pTask->deps[lastIdx].name + nameCopyLen;
        memcpy(pTask->deps[lastIdx].name, dep->name, nameCopyLen);
        memcpy(pTask->deps[lastIdx].event, dep->event, eventCopyLen);
        pthread_mutex_unlock(&ctx->lock);
        return 0;
    }
    pthread_mutex_unlock(&ctx->lock);
    crinitErrPrint("Could not find task \'%s\' in TaskDB.", taskName);
    return -1;
}

int crinitTaskDBRemoveDepFromTask(crinitTaskDB_t *ctx, const crinitTaskDep_t *dep, const char *taskName) {
    crinitNullCheck(-1, ctx, dep, taskName);

    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }

    crinitTask_t *pTask;
    if (crinitFindTask(&pTask, taskName, ctx) == 0) {
        for (size_t j = 0; j < pTask->depsSize; j++) {
            if ((strcmp(pTask->deps[j].name, dep->name) == 0) && (strcmp(pTask->deps[j].event, dep->event) == 0)) {
                crinitDbgInfoPrint("Removing dependency \'%s:%s\' in \'%s\'.", dep->name, dep->event, pTask->name);
                free(pTask->deps[j].name);
                if (j < pTask->depsSize - 1) {
                    pTask->deps[j] = pTask->deps[pTask->depsSize - 1];
                }
                pTask->depsSize--;
            }
        }
        pthread_cond_broadcast(&ctx->changed);
        pthread_mutex_unlock(&ctx->lock);
        return 0;
    }
    pthread_mutex_unlock(&ctx->lock);
    crinitErrPrint("Could not find task \'%s\' in TaskDB.", taskName);
    return -1;
}

int crinitTaskDBFulfillDep(crinitTaskDB_t *ctx, const crinitTaskDep_t *dep, const crinitTask_t *target) {
    crinitNullCheck(-1, ctx, dep);

    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }

    crinitTask_t *pTask;
    crinitTaskDbForEach(ctx, pTask) {
        if (target != NULL && pTask != target) {
            continue;
        }

        for (size_t j = 0; j < pTask->depsSize; j++) {
            if ((strcmp(pTask->deps[j].name, dep->name) == 0) && (strcmp(pTask->deps[j].event, dep->event) == 0)) {
                crinitDbgInfoPrint("Removing fulfilled dependency \'%s:%s\' in \'%s\'.", dep->name, dep->event,
                                   pTask->name);
                free(pTask->deps[j].name);
                if (j < pTask->depsSize - 1) {
                    pTask->deps[j] = pTask->deps[pTask->depsSize - 1];
                }
                pTask->depsSize--;
            }
        }
    }
    pthread_cond_broadcast(&ctx->changed);
    pthread_mutex_unlock(&ctx->lock);
    return 0;
}

int crinitTaskDBProvideFeature(crinitTaskDB_t *ctx, const crinitTask_t *provider, crinitTaskState_t newState) {
    crinitNullCheck(-1, ctx, provider);

    for (size_t i = 0; i < provider->prvSize; i++) {
        if (provider->prv[i].stateReq == newState) {
            char depName[] = CRINIT_PROVIDE_DEP_NAME;
            const crinitTaskDep_t dep = {depName, provider->prv[i].name};
            if (crinitTaskDBFulfillDep(ctx, &dep, NULL) == -1) {
                crinitErrPrint("Could not fulfill dependency \'%s:%s\'.", dep.name, dep.event);
                return -1;
            }
            crinitDbgInfoPrint("Fulfilled feature dependency \'%s:%s\'.", dep.name, dep.event);
            if (crinitFeatureHook(dep.event, CRINIT_HOOK_START, (void *)ctx) == -1) {
                crinitErrPrint("Could not run activation hook for feature \'%s\'.", dep.event);
                return -1;
            }
        } else {
            if (crinitFeatureHook(provider->prv[i].name, CRINIT_HOOK_STOP, (void *)ctx) == -1) {
                crinitErrPrint("Could not run deactivation hook for feature \'%s\'.", provider->prv[i].name);
                return -1;
            }
        }
    }
    return 0;
}

int crinitTaskDBProvideFeatureByTaskName(crinitTaskDB_t *ctx, const char *taskName, crinitTaskState_t newState) {
    crinitNullCheck(-1, ctx, taskName);

    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }

    crinitTask_t *provider;
    if (crinitFindTask(&provider, taskName, ctx) == -1) {
        crinitErrPrint("Could not find task \'%s\' in TaskDB.", taskName);
        return -1;
    }
    pthread_mutex_unlock(&ctx->lock);

    return crinitTaskDBProvideFeature(ctx, provider, newState);
}

int crinitTaskDBExportTaskNamesToArray(crinitTaskDB_t *ctx, char **tasks[], size_t *numTasks) {
    int ret = 0;
    size_t i;

    crinitNullCheck(-1, ctx, tasks, numTasks);

    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }

    *numTasks = 0;
    *tasks = calloc(ctx->taskSetItems, sizeof(*tasks));
    if (*tasks == NULL) {
        crinitErrnoPrint("Could not allocate memory for task array.");
        pthread_mutex_unlock(&ctx->lock);
        return -1;
    }

    for (i = 0; i < ctx->taskSetItems; i++) {
        (*tasks)[i] = strdup(ctx->taskSet[i].name);
        if ((*tasks)[i] == NULL) {
            crinitErrnoPrint("Could not allocate memory for task name.");
            ret = -1;
            break;
        }
    }

    if (ret == 0) {
        *numTasks = i;
    } else {
        for (size_t j = 0; j < i; j++) {
            free((*tasks)[i]);
        }
        free(*tasks);
        *tasks = NULL;
    }

    pthread_mutex_unlock(&ctx->lock);
    return ret;
}

static int crinitFindTask(crinitTask_t **task, const char *taskName, const crinitTaskDB_t *in) {
    crinitNullCheck(-1, taskName, in);

    crinitTask_t *pTask;
    crinitTaskDbForEach(in, pTask) {
        if (strcmp(taskName, pTask->name) == 0) {
            *task = pTask;
            return 0;
        }
    }

    *task = NULL;

    return -1;
}

static bool crinitTaskIsReady(const crinitTask_t *t) {
    crinitNullCheck(false, t);

    if (t->depsSize != 0) {
        return false;
    }
    if (t->state & (CRINIT_TASK_STATE_RUNNING | CRINIT_TASK_STATE_STARTING)) {
        return false;
    }
    if (t->state & (CRINIT_TASK_STATE_FAILED | CRINIT_TASK_STATE_DONE)) {
        if (!(t->opts & CRINIT_TASK_OPT_RESPAWN)) {
            return false;
        }
        if (t->maxRetries != -1 && t->failCount > t->maxRetries) {
            return false;
        }
    }
    return true;
}
