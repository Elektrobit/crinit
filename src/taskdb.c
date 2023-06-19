/**
 * @file taskdb.c
 * @brief Implementation of the central Task Database and related functions.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "taskdb.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "globopt.h"
#include "logio.h"
#include "optfeat.h"

/**
 * Find index of a task in the crinitTaskDB_t::taskSet of an crinitTaskDB_t by name.
 *
 * @param idx       Pointer to return the index with.
 * @param taskName  The crinitTask_t::name to search for.
 * @param in        The crinitTaskDB_t_t to search in.
 *
 * @return 0 on success, -1 otherwise
 */
static int EBCL_findInTaskDB(ssize_t *idx, const char *taskName, const crinitTaskDB_t *in);
/**
 * Check if an crinitTask_t is considered ready to be started (startable).
 *
 * See crinitTaskDBSpawnReady() for further explanation.
 *
 * @param t The task to be checked for readiness.
 *
 * @return true if \a t is ready, false otherwise
 */
static bool EBCL_taskReady(const crinitTask_t *t);

int crinitTaskDBInitWithSize(crinitTaskDB_t *ctx, int (*spawnFunc)(crinitTaskDB_t *ctx, const crinitTask_t *),
                            size_t initialSize) {
    if (ctx == NULL) {
        crinitErrPrint("Given crinitTaskDB_t to initialize must not be NULL.");
        return -1;
    }
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
    if (ctx == NULL) {
        crinitErrPrint("Given crinitTaskDB_t to destroy must not be NULL.");
        return -1;
    }
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

    ssize_t idx = -1;
    if (EBCL_findInTaskDB(&idx, t->name, ctx) == 0) {
        if (overwrite) {
            crinitDestroyTask(&ctx->taskSet[idx]);
        } else {
            crinitErrPrint("Found task/include with name '%s' already in TaskDB but will not overwrite", t->name);
            goto fail;
        }
    }
    if (idx == -1 && ctx->taskSetItems == ctx->taskSetSize) {
        // We need to grow the backing array
        crinitTask_t *newSet = realloc(ctx->taskSet, ctx->taskSetSize * 2 * sizeof(crinitTask_t));
        if (newSet == NULL) {
            crinitErrnoPrint("Could not allocate additional memory for more task/include elements.");
            goto fail;
        }
        ctx->taskSet = newSet;
        ctx->taskSetSize *= 2;
    }
    if (idx == -1) {
        idx = ctx->taskSetItems++;
    }

    crinitTask_t *tempDuplicate = NULL;
    if (crinitTaskDup(&tempDuplicate, t) == -1) {
        crinitErrPrint("Could not duplicate new Task into temporary variable.");
        goto fail;
    }
    memcpy(&ctx->taskSet[idx], tempDuplicate, sizeof(crinitTask_t));
    free(tempDuplicate);

    pthread_cond_broadcast(&ctx->changed);
    pthread_mutex_unlock(&ctx->lock);
    return 0;
fail:
    pthread_mutex_unlock(&ctx->lock);
    return -1;
}

int crinitTaskDBSpawnReady(crinitTaskDB_t *ctx) {
    if (ctx == NULL) {
        crinitErrPrint("The TaskDB context must not be NULL.");
        return -1;
    }
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

    for (size_t i = 0; i < ctx->taskSetItems; i++) {
        crinitTask_t *pTask = &ctx->taskSet[i];
        if (EBCL_taskReady(pTask)) {
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
    if (ctx == NULL) {
        crinitErrPrint("The TaskDB context must not be NULL.");
        return -1;
    }
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
    if (ctx == NULL || taskName == NULL) {
        crinitErrPrint("The TaskDB context and the taskName must not be NULL.");
        return -1;
    }

    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }
    for (size_t i = 0; i < ctx->taskSetItems; i++) {
        crinitTask_t *pTask = &ctx->taskSet[i];
        if (strcmp(pTask->name, taskName) == 0) {
            pTask->state = s;
            s &= ~CRINIT_TASK_STATE_NOTIFIED;  // Here we don't care if we got the state via notification or directly.
            if (s == CRINIT_TASK_STATE_FAILED) {
                pTask->failCount++;
            } else if (s == CRINIT_TASK_STATE_DONE) {
                pTask->failCount = 0;
            }
            pthread_cond_broadcast(&ctx->changed);
            pthread_mutex_unlock(&ctx->lock);
            return 0;
        }
    }
    pthread_mutex_unlock(&ctx->lock);
    crinitErrPrint("Could not set TaskState for Task \'%s\' as it does not exist in TaskDB.", taskName);
    return -1;
}

int crinitTaskDBGetTaskState(crinitTaskDB_t *ctx, crinitTaskState_t *s, const char *taskName) {
    if (ctx == NULL || taskName == NULL || s == NULL) {
        crinitErrPrint("The TaskDB context, taskName, and result pointer must not be NULL.");
        return -1;
    }
    *s = 0;
    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }
    for (size_t i = 0; i < ctx->taskSetItems; i++) {
        crinitTask_t *pTask = &ctx->taskSet[i];
        if (strcmp(pTask->name, taskName) == 0) {
            *s = pTask->state;
            pthread_mutex_unlock(&ctx->lock);
            return 0;
        }
    }
    pthread_mutex_unlock(&ctx->lock);
    crinitErrPrint("Could not get TaskState of Task \'%s\' as it does not exist in TaskDB.", taskName);
    return -1;
}

int crinitTaskDBSetTaskPID(crinitTaskDB_t *ctx, pid_t pid, const char *taskName) {
    if (ctx == NULL || taskName == NULL) {
        crinitErrPrint("The TaskDB context and the taskName must not be NULL.");
        return -1;
    }

    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }
    for (size_t i = 0; i < ctx->taskSetItems; i++) {
        crinitTask_t *pTask = &ctx->taskSet[i];
        if (strcmp(pTask->name, taskName) == 0) {
            pTask->pid = pid;
            pthread_mutex_unlock(&ctx->lock);
            return 0;
        }
    }
    pthread_mutex_unlock(&ctx->lock);
    crinitErrPrint("Could not set TaskState for Task \'%s\' as it does not exist in TaskDB.", taskName);
    return -1;
}

int crinitTaskDBGetTaskPID(crinitTaskDB_t *ctx, pid_t *pid, const char *taskName) {
    if (ctx == NULL || taskName == NULL || pid == NULL) {
        crinitErrPrint("The TaskDB context, taskName, and result pointer must not be NULL.");
        return -1;
    }
    *pid = -1;
    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }
    for (size_t i = 0; i < ctx->taskSetItems; i++) {
        crinitTask_t *pTask = &ctx->taskSet[i];
        if (strcmp(pTask->name, taskName) == 0) {
            *pid = pTask->pid;
            pthread_mutex_unlock(&ctx->lock);
            return 0;
        }
    }
    pthread_mutex_unlock(&ctx->lock);
    crinitErrPrint("Could not get TaskState of Task \'%s\' as it does not exist in TaskDB.", taskName);
    return -1;
}

int crinitTaskDBGetTaskStateAndPID(crinitTaskDB_t *ctx, crinitTaskState_t *s, pid_t *pid, const char *taskName) {
    if (ctx == NULL || taskName == NULL || s == NULL || pid == NULL) {
        crinitErrPrint("The TaskDB context, taskName, and result pointers must not be NULL.");
        return -1;
    }
    *s = 0;
    *pid = 0;
    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }
    for (size_t i = 0; i < ctx->taskSetItems; i++) {
        crinitTask_t *pTask = &ctx->taskSet[i];
        if (strcmp(pTask->name, taskName) == 0) {
            *s = pTask->state;
            *pid = pTask->pid;
            pthread_mutex_unlock(&ctx->lock);
            return 0;
        }
    }
    pthread_mutex_unlock(&ctx->lock);
    crinitErrPrint("Could not get TaskState of Task \'%s\' as it does not exist in TaskDB.", taskName);
    return -1;
}

int crinitTaskDBAddDepToTask(crinitTaskDB_t *ctx, const crinitTaskDep_t *dep, const char *taskName) {
    if (ctx == NULL || dep == NULL || taskName == NULL) {
        crinitErrPrint("The TaskDB context, the TaskDep to add, and the task name to search for must not be NULL.");
        return -1;
    }
    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }
    for (size_t i = 0; i < ctx->taskSetItems; i++) {
        crinitTask_t *pTask = &ctx->taskSet[i];
        if (strcmp(pTask->name, taskName) == 0) {
            // Return immediately if dependency is already present
            for (size_t j = 0; j < pTask->depsSize; j++) {
                if (strcmp(pTask->deps[j].name, dep->name) == 0 && strcmp(pTask->deps[j].event, dep->event) == 0) {
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
    }
    pthread_mutex_unlock(&ctx->lock);
    crinitErrPrint("Could not find task \'%s\' in TaskDB.", taskName);
    return -1;
}

int crinitTaskDBRemoveDepFromTask(crinitTaskDB_t *ctx, const crinitTaskDep_t *dep, const char *taskName) {
    if (ctx == NULL || dep == NULL || taskName == NULL) {
        crinitErrPrint("The TaskDB context, the TaskDep to remove, and the task name to search for must not be NULL.");
        return -1;
    }

    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }

    for (size_t i = 0; i < ctx->taskSetItems; i++) {
        crinitTask_t *pTask = &ctx->taskSet[i];
        if (strcmp(pTask->name, taskName) == 0) {
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
    }
    pthread_mutex_unlock(&ctx->lock);
    crinitErrPrint("Could not find task \'%s\' in TaskDB.", taskName);
    return -1;
}

int crinitTaskDBFulfillDep(crinitTaskDB_t *ctx, const crinitTaskDep_t *dep) {
    if (ctx == NULL || dep == NULL) {
        crinitErrPrint("The TaskDB context and the TaskDep to fulfill must not be NULL.");
        return -1;
    }

    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }

    for (size_t i = 0; i < ctx->taskSetItems; i++) {
        crinitTask_t *pTask = &ctx->taskSet[i];
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
    if (ctx == NULL || provider == NULL) {
        crinitErrPrint("The TaskDB context and the feature-providing task must not be NULL.");
        return -1;
    }

    for (size_t i = 0; i < provider->prvSize; i++) {
        if (provider->prv[i].stateReq == newState) {
            char depName[] = CRINIT_PROVIDE_DEP_NAME;
            const crinitTaskDep_t dep = {depName, provider->prv[i].name};
            if (crinitTaskDBFulfillDep(ctx, &dep) == -1) {
                crinitErrPrint("Could not fulfill dependency \'%s:%s\'.", dep.name, dep.event);
                return -1;
            }
            crinitDbgInfoPrint("Fulfilled feature dependency \'%s:%s\'.", dep.name, dep.event);
            if (EBCL_crinitFeatureHook(dep.event) == -1) {
                crinitErrPrint("Could not run activiation hook for feature \'%s\'.", dep.event);
                return -1;
            }
        }
    }
    return 0;
}

int crinitTaskDBProvideFeatureByTaskName(crinitTaskDB_t *ctx, const char *taskName, crinitTaskState_t newState) {
    if (ctx == NULL || taskName == NULL) {
        crinitErrPrint("The TaskDB context and the name of the feature-providing task must not be NULL.");
        return -1;
    }

    const crinitTask_t *provider = NULL;

    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }
    ssize_t taskIdx;
    if (EBCL_findInTaskDB(&taskIdx, taskName, ctx) == -1) {
        crinitErrPrint("Could not find task \'%s\' in TaskDB.", taskName);
        return -1;
    }
    provider = &ctx->taskSet[taskIdx];
    pthread_mutex_unlock(&ctx->lock);

    return crinitTaskDBProvideFeature(ctx, provider, newState);
}

int crinitTaskDBExportTaskNamesToArray(crinitTaskDB_t *ctx, char **tasks[], size_t *numTasks) {
    int ret = 0;

    if (ctx == NULL || tasks == NULL || numTasks == NULL) {
        crinitErrPrint("The TaskDB context and the output array pointer must not be NULL.");
        return -1;
    }

    if ((errno = pthread_mutex_lock(&ctx->lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }

    *numTasks = 0;
    *tasks = malloc(ctx->taskSetItems * sizeof(*tasks));
    if (*tasks == NULL) {
        crinitErrnoPrint("Could not allocate memory for task array.");
        pthread_mutex_unlock(&ctx->lock);
        return -1;
    }

    for (size_t i = 0; i < ctx->taskSetItems; i++) {
        crinitTask_t *pTask = &ctx->taskSet[i];
        (*tasks)[i] = strdup(pTask->name);
        if ((*tasks)[i] == NULL) {
            crinitErrnoPrint("Could not allocate memory for task name.");
            ret = -1;
            goto fail;
        }
        (*numTasks)++;
    }

    goto success;
fail:
    for (size_t i = 0; i < *numTasks; i++) {
        free((*tasks)[i]);
    }
    free(*tasks);
    *tasks = NULL;
    *numTasks = 0;
success:
    pthread_mutex_unlock(&ctx->lock);
    return ret;
}

static int EBCL_findInTaskDB(ssize_t *idx, const char *taskName, const crinitTaskDB_t *in) {
    if (taskName == NULL || in == NULL) {
        return -1;
    }

    for (size_t i = 0; i < in->taskSetItems; i++) {
        if (strcmp(taskName, in->taskSet[i].name) == 0) {
            if (idx != NULL) {
                *idx = i;
            }
            return 0;
        }
    }
    if (idx != NULL) {
        *idx = -1;
    }
    return -1;
}

static bool EBCL_taskReady(const crinitTask_t *t) {
    if (t == NULL) {
        return false;
    }
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
