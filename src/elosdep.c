// SPDX-License-Identifier: MIT
/**
 * @file elosdep.c
 * @brief Implementation of elos connection.
 */
#include "elosdep.h"

#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>

#include "common.h"
#include "elos-common.h"
#include "list.h"
#include "logio.h"
#include "task.h"
#include "taskdb.h"

#define CRINIT_ELOS_IDENT "crinit"  ///< Identification string for crinit logging to syslog
/* HINT: We are relying on the major library version here. */
#define CRINIT_ELOS_DEPENDENCY "@elos"  ///< Elos filter dependency prefix

static bool crinitElosActivated = false;  ///< Indicates if the elos connection and handler thread has been set up.
static pthread_mutex_t crinitElosActivatedLock = PTHREAD_MUTEX_INITIALIZER;  ///< Mutex to guard crinitElosActivated.

/**
 * Task that has unfulfilled filter dependencies.
 */
typedef struct crinitElosdepFilterTask_t {
    crinitTask_t *task;          ///< The monitored task
    crinitList_t filterList;     ///< List unfulfilled filter dependencies
    pthread_mutex_t filterLock;  ///< Lock protecting the list of filters
    crinitList_t list;           ///< List handle for filter task list
} crinitElosdepFilterTask_t;

/**
 * Definition of a single filter related to a task.
 */
typedef struct crinitElosdepFilter_t {
    char *name;                             ///< Name of the filter
    char *filter;                           ///< The filter rule string
    crinitElosEventQueueId_t eventQueueId;  ///< ID of the elos event queue related to this filter
    crinitList_t list;                      ///< List handle for filter list
} crinitElosdepFilter_t;

/**
 * Thread context of the elosdep main thread and elos vtable.
 */
static struct crinitElosEventThread_t {
    pthread_t threadId;            ///< Thread identifier
    bool elosStarted;              ///< Wether or not an initial conenction to elos has been established
    crinitTaskDB_t *taskDb;        ///< Pointer to crinit task database
    crinitElosSession_t *session;  ///< Elos session handle
} crinitTinfo;

/** List of tasks with elos filter dependencies **/
static crinitList_t crinitFilterTasks = CRINIT_LIST_INIT(crinitFilterTasks);

/** Mutex synchronizing elos filter task registration **/
static pthread_mutex_t crinitElosdepFilterTaskLock = PTHREAD_MUTEX_INITIALIZER;

/** Mutex synchronizing elos connection **/
static pthread_mutex_t crinitElosdepSessionLock = PTHREAD_MUTEX_INITIALIZER;

/**
 * Frees the heap allocated members of the filter.
 *
 * @param filter Filter to be destroyed.
 */
static void crinitElosdepFilterDestroy(crinitElosdepFilter_t *filter) {
    free(filter->name);
    free(filter->filter);
    free(filter);
}

/**
 * Inserts an elos filter into the list of filter subscriptions.
 *
 * @param filterTask  Task to register the filter for.
 * @param filter      Filter to be registered.
 *
 * @return Returns 0 if the filter has been inserted, -1 otherwise.
 */
static int crinitElosdepFilterRegister(crinitElosdepFilterTask_t *filterTask, crinitElosdepFilter_t *filter) {
    if ((errno = pthread_mutex_lock(&filterTask->filterLock)) != 0) {
        crinitErrnoPrint("Failed to lock elos filter list.");
        return -1;
    }

    crinitListAppend(&filterTask->filterList, &filter->list);

    if ((errno = pthread_mutex_unlock(&filterTask->filterLock)) != 0) {
        crinitErrnoPrint("Failed to unlock elos filter list.");
        return -1;
    }

    return 0;
}

/**
 * Removes the given filter from the filter list.
 *
 * @param filterTask  Task to remove the filter from.
 * @param filter      Filter to be destroyed.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
static int crinitElosdepFilterUnregister(crinitElosdepFilterTask_t *filterTask, crinitElosdepFilter_t *filter) {
    if ((errno = pthread_mutex_lock(&filterTask->filterLock)) != 0) {
        crinitErrnoPrint("Failed to lock elos filter list.");
        return -1;
    }

    crinitListDelete(&filter->list);
    crinitElosdepFilterDestroy(filter);

    if ((errno = pthread_mutex_unlock(&filterTask->filterLock)) != 0) {
        crinitErrnoPrint("Failed to unlock elos filter list.");
        return -1;
    }

    return 0;
}

/**
 * Free the complete list of subscribed filters.
 *
 * @param filterTask The task owning this filter list.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
static int crinitElosdepFilterListClear(crinitElosdepFilterTask_t *filterTask) {
    crinitElosdepFilter_t *cur, *temp;

    if ((errno = pthread_mutex_lock(&filterTask->filterLock)) != 0) {
        crinitErrnoPrint("Failed to lock elos filter list.");
        return -1;
    }

    crinitListForEachEntrySafe(cur, temp, &filterTask->filterList, list) {
        crinitListDelete(&cur->list);
        crinitElosdepFilterDestroy(cur);
    }

    if ((errno = pthread_mutex_unlock(&filterTask->filterLock)) != 0) {
        crinitErrnoPrint("Failed to unlock elos filter list.");
        return -1;
    }

    return 0;
}

/**
 * Frees the heap allocated members of the filter task.
 *
 * @param filterTask Filter task to be destroyed.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
static int crinitElosdepFilterTaskDestroy(crinitElosdepFilterTask_t *filterTask) {
    int res = crinitElosdepFilterListClear(filterTask);
    if (res == 0) {
        free(filterTask);
    }

    return res;
}

/**
 * Free the complete list of filter tasks.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
static int crinitElosdepFilterTaskListClear(void) {
    int res = 0;
    crinitElosdepFilterTask_t *cur, *temp;

    /* Insert into list of filters of filter task */
    if ((errno = pthread_mutex_lock(&crinitElosdepFilterTaskLock)) != 0) {
        crinitErrnoPrint("Failed to lock elos filter task list.");
        return -1;
    }

    crinitListForEachEntrySafe(cur, temp, &crinitFilterTasks, list) {
        crinitListDelete(&cur->list);
        if ((res = crinitElosdepFilterTaskDestroy(cur)) != 0) {
            crinitErrnoPrint("Failed to lock elos filter task list.");
            res = -1;
            break;
        }
    }

    if ((errno = pthread_mutex_unlock(&crinitElosdepFilterTaskLock)) != 0) {
        crinitErrnoPrint("Failed to unlock elos filter task list.");
        return -1;
    }

    return res;
}

/**
 * Create and register a new elos event filter handle from a given environment set.
 *
 * @param task   Task to create elos filter for.
 * @param name   Name of the filter to register.
 * @param filter Pointer to the created filter.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
static int crinitElosdepFilterFromEnvSet(const crinitTask_t *task, const char *name, crinitElosdepFilter_t **filter) {
    int res = -1;
    crinitElosdepFilter_t *temp;
    const crinitEnvSet_t *es = &task->elosFilters;
    char *out;

    if (strchr(name, '=') != NULL) {
        crinitErrPrint("Environment variable names must not contain '='.");
        return res;
    }

    size_t cmpLen = strlen(name);

    for (size_t i = 0; es->envp[i] != NULL; i++) {
        out = strchr(es->envp[i], '=');
        if (out == NULL) {
            continue;
        }

        if (strncmp(es->envp[i], name, cmpLen) != 0 || es->envp[i][cmpLen] != '=') {
            continue;
        }

        temp = malloc(sizeof(*temp));
        if (temp == NULL) {
            crinitErrPrint("Failed to allocate memory for the elos filter.");
            break;
        }

        temp->name = strndup(es->envp[i], cmpLen);
        temp->filter = strdup(es->envp[i] + cmpLen + 1);
        temp->eventQueueId = ELOS_ID_INVALID;

        *filter = temp;
        res = 0;
        break;
    }

    return res;
}

/**
 * Subscribes the filter with elos.
 *
 * @param filter The filter to subscribe.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
static inline int crinitElosdepFilterSubscribe(crinitElosdepFilter_t *filter) {
    crinitDbgInfoPrint("Try to subscribe with filter %s: %s\n", filter->name, filter->filter);
    return crinitElosTryExec(crinitTinfo.session, &crinitElosdepSessionLock, crinitElosGetVTable()->eventSubscribe,
                             "Failed to subscribe with filter.", crinitTinfo.session, (const char **)&filter->filter, 1,
                             &filter->eventQueueId);
}

/**
 * Subscribes all filters currently registered with elosdep.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
static int crinitElosdepFilterListSubscribe(void) {
    int res = 0;
    crinitElosdepFilter_t *filter;
    crinitElosdepFilterTask_t *filterTask;

    if ((errno = pthread_mutex_lock(&crinitElosdepFilterTaskLock)) != 0) {
        crinitErrnoPrint("Failed to lock elos filter task list.");
        return -1;
    }

    crinitListForEachEntry(filterTask, &crinitFilterTasks, list) {
        crinitListForEachEntry(filter, &filterTask->filterList, list) {
            if ((res = crinitElosdepFilterSubscribe(filter)) != 0) {
                crinitErrPrint("Failed to subscribe filter.");
                goto err;
            }
        }
    }

err:
    if ((errno = pthread_mutex_unlock(&crinitElosdepFilterTaskLock)) != 0) {
        crinitErrnoPrint("Failed to unlock elos filter list.");
        res = -1;
    }

    return res;
}

/**
 * Unsubscribes the filter from elos.
 *
 * @param filter The filter to unsubscribe.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
static inline int crinitElosdepFilterUnsubscribe(crinitElosdepFilter_t *filter) {
    return crinitElosTryExec(crinitTinfo.session, &crinitElosdepSessionLock, crinitElosGetVTable()->eventUnsubscribe,
                             "Failed to unsubscribe filter.", crinitTinfo.session, filter->eventQueueId);
}

/**
 * Extracts all filter defines for each task and creates a local
 * elos filter handle, which will be used to subscribe for this filter.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
static int crinitElosdepFilterListUnsubscribe(void) {
    int res = 0;
    crinitElosdepFilter_t *filter;
    crinitElosdepFilterTask_t *filterTask;

    if ((errno = pthread_mutex_lock(&crinitElosdepFilterTaskLock)) != 0) {
        crinitErrnoPrint("Failed to lock elos filter task list.");
        return -1;
    }

    crinitListForEachEntry(filterTask, &crinitFilterTasks, list) {
        crinitListForEachEntry(filter, &filterTask->filterList, list) {
            if ((res = crinitElosdepFilterUnsubscribe(filter)) != 0) {
                crinitErrPrint("Failed to subscribe filter.");
                goto err;
            }
        }
    }

err:
    if ((errno = pthread_mutex_unlock(&crinitElosdepFilterTaskLock)) != 0) {
        crinitErrnoPrint("Failed to unlock elos filter list.");
        res = -1;
    }

    return res;
}

int crinitElosdepTaskAdded(crinitTask_t *task) {
    int res = 0;
    crinitTaskDep_t *dep;
    crinitElosdepFilter_t *filter = NULL;
    crinitElosdepFilterTask_t *filterTask = NULL;

    crinitNullCheck(-1, task);

    crinitDbgInfoPrint("Scanning task %s for elos dependencies.", task->name);
    crinitTaskForEachDep(task, dep) {
        if (strcmp(dep->name, CRINIT_ELOS_DEPENDENCY) != 0) {
            continue;
        }

        crinitDbgInfoPrint("Found elos dependency %s:%s in task %s.", dep->name, dep->event, task->name);
        if (filterTask == NULL) {
            filterTask = malloc(sizeof(*filterTask));
            if (filterTask == NULL) {
                crinitErrPrint("Failed to allocate memory for the elos filter task.");
                res = -1;
                break;
            }

            filterTask->task = task;
            if (pthread_mutex_init(&filterTask->filterLock, NULL) != 0) {
                crinitErrnoPrint("Failed to initialize filter list lock.");
                res = -1;
                break;
            }

            crinitListInit(&filterTask->filterList);

            if ((errno = pthread_mutex_lock(&crinitElosdepFilterTaskLock)) != 0) {
                crinitErrnoPrint("Failed to lock elos filter task list.");
                res = -1;
                break;
            }

            crinitListAppend(&crinitFilterTasks, &filterTask->list);

            if ((errno = pthread_mutex_unlock(&crinitElosdepFilterTaskLock)) != 0) {
                crinitErrnoPrint("Failed to unlock elos filter task list.");
                res = -1;
                break;
            }
        }

        crinitDbgInfoPrint("Searching for filter for dependency %s:%s.", dep->name, dep->event);
        if ((res = crinitElosdepFilterFromEnvSet(task, dep->event, &filter)) != 0) {
            crinitErrPrint("Failed to find filter for dependency %s:%s.", dep->name, dep->event);
            break;
        }

        if ((res = crinitElosdepFilterRegister(filterTask, filter)) != 0) {
            crinitErrPrint("Failed to register filter for dependency %s:%s.", dep->name, dep->event);
            crinitElosdepFilterDestroy(filter);
            break;
        }

        /* Subscribing the filter might fail if elos is not started yet */
        if (crinitTinfo.elosStarted && (res = crinitElosdepFilterSubscribe(filter)) != 0) {
            crinitErrPrint("Failed to subscribe filter for dependency %s:%s.", dep->name, dep->event);
            break;
        }
    }

    return res;
}

static void *crinitElosdepEventListener(void *arg) {
    int err = -1;
    const char *version;
    crinitElosdepFilter_t *filter, *tempFilter;
    crinitElosdepFilterTask_t *filterTask;
    crinitElosEventVector_t *eventVector = NULL;

    struct crinitElosEventThread_t *tinfo = arg;

    tinfo->elosStarted = true;

    err = crinitElosTryExec(crinitTinfo.session, &crinitElosdepSessionLock, crinitElosGetVTable()->getVersion,
                            "Failed to request elos version.", tinfo->session, &version);
    if (err == SAFU_RESULT_OK) {
        crinitInfoPrint("Connected to elosd version %s for event reception.", version);
    } else {
        goto err_connection_lost;
    }

    if ((err = crinitElosdepFilterListSubscribe()) != 0) {
        crinitErrnoPrint("Failed to subscribe already registered elos filters.");
        goto err_connection_lost;
    }

    while (1) {
        if ((errno = pthread_mutex_lock(&crinitElosActivatedLock)) != 0) {
            crinitErrnoPrint("Failed to lock elos connection activation indicator.");
            goto err_connection_lost;
        }
        if (!crinitElosActivated) {
            if ((errno = pthread_mutex_unlock(&crinitElosActivatedLock)) != 0) {
                crinitErrnoPrint("Failed to unlock elos connection activation indicator.");
            }
            goto err_connection_lost;
        }
        if ((errno = pthread_mutex_unlock(&crinitElosActivatedLock)) != 0) {
            crinitErrnoPrint("Failed to unlock elos connection activation indicator.");
            goto err_connection_lost;
        }

        crinitListForEachEntry(filterTask, &crinitFilterTasks, list) {
            crinitListForEachEntrySafe(filter, tempFilter, &filterTask->filterList, list) {
                err = crinitElosTryExec(crinitTinfo.session, &crinitElosdepSessionLock,
                                        crinitElosGetVTable()->eventQueueRead, "Failed to read elos event queue.",
                                        tinfo->session, filter->eventQueueId, &eventVector);
                if (err == SAFU_RESULT_OK && eventVector && eventVector->elementCount > 0) {
                    const crinitTaskDep_t taskDep = {
                        .name = CRINIT_ELOS_DEPENDENCY,
                        .event = filter->name,
                    };

                    if ((err = crinitTaskDBFulfillDep(tinfo->taskDb, &taskDep, filterTask->task)) != 0) {
                        crinitErrnoPrint("Failed to fulfill dependency %s:%s.", taskDep.name, taskDep.event);
                        goto err_session;
                    }

                    if ((err = crinitElosdepFilterUnregister(filterTask, filter)) != 0) {
                        crinitErrnoPrint("Failed to remove filter from dependency list.");
                        goto err_session;
                    }
                }
            }
        }

        usleep(500000);
    }

err_connection_lost:
    if ((err = crinitElosdepFilterTaskListClear()) != 0) {
        crinitErrnoPrint("Failed to clear filter tasks.");
    }
    if ((errno = pthread_mutex_lock(&crinitElosdepSessionLock)) != 0) {
        crinitErrnoPrint("Failed to lock elos session.");
        return NULL;
    }
    free(crinitTinfo.session);
    crinitTinfo.session = NULL;
    crinitTinfo.elosStarted = false;
    if ((errno = pthread_mutex_unlock(&crinitElosdepSessionLock)) != 0) {
        crinitErrnoPrint("Failed to unlock elos session.");
    }

    return NULL;

err_session:
    if ((err = crinitElosdepFilterListUnsubscribe()) != 0) {
        crinitErrnoPrint("Failed to unsubscribe elos filters.");
    }

    if ((err = crinitElosdepFilterTaskListClear()) != 0) {
        crinitErrnoPrint("Failed to clear filter tasks.");
    }

    crinitElosDisconnect(crinitTinfo.session, &crinitElosdepSessionLock);

    return NULL;
}

/**
 * Initializes the vtable managed within the elosdep thread context.
 *
 * @param taskDb Pointer to the crinit task database
 * @param tinfo Elosdep thread context
 * @return Returns 0 on success, -1 otherwise.
 */
static int crinitElosdepInitThreadContext(crinitTaskDB_t *taskDb, struct crinitElosEventThread_t *tinfo) {
    tinfo->taskDb = taskDb;

    crinitElosInit();

    return 0;
}

int crinitElosdepActivate(crinitTaskDB_t *taskDb, bool e) {
    int res;

    if ((errno = pthread_mutex_lock(&crinitElosActivatedLock)) != 0) {
        crinitErrnoPrint("Failed to lock elos connection activation indicator.");
        return -1;
    }
    if (e && !crinitElosActivated) {
        if (crinitElosdepInitThreadContext(taskDb, &crinitTinfo) != 0) {
            crinitErrPrint("Failed to load elos interface.");
            return -1;
        }

        pthread_attr_t attrs;
        res = pthread_attr_init(&attrs);
        if (res != 0) {
            crinitErrPrint("Could not initialize pthread attributes.");
            return -1;
        }

        res = pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
        if (res != 0) {
            crinitErrPrint("Could not set PTHREAD_CREATE_DETACHED attribute.");
            pthread_attr_destroy(&attrs);
            return -1;
        }

        res = pthread_attr_setstacksize(&attrs, CRINIT_THREADPOOL_THREAD_STACK_SIZE);
        if (res != 0) {
            crinitErrPrint("Could not set pthread stack size to %d.", CRINIT_THREADPOOL_THREAD_STACK_SIZE);
            pthread_attr_destroy(&attrs);
            return -1;
        }

        crinitDbgInfoPrint("Starting elos event handler thread.");
        res = pthread_create(&crinitTinfo.threadId, &attrs, crinitElosdepEventListener, (void *)&crinitTinfo);
        if (res != 0) {
            crinitErrPrint("Could not create elos event handler thread.");
            pthread_attr_destroy(&attrs);
            return -1;
        }

        res = pthread_attr_destroy(&attrs);
        if (res != 0) {
            crinitErrPrint("Failed to free thread attributes.");
            return -1;
        }
    }

    crinitElosActivated = e;

    if ((errno = pthread_mutex_unlock(&crinitElosActivatedLock)) != 0) {
        crinitErrnoPrint("Failed to unlock elos connection activation indicator.");
        return -1;
    }
    return 0;
}
