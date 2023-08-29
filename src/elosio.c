// SPDX-License-Identifier: MIT
/**
 * @file elosio.c
 * @brief Implementation of elos connection.
 */
#include "elosio.h"

#include <dlfcn.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "confparse.h"
#include "globopt.h"
#include "list.h"
#include "logio.h"
#include "task.h"
#include "taskdb.h"
#include "thrpool.h"

#define CRINIT_ELOS_IDENT "crinit"  ///< Identification string for crinit logging to syslog
/* HINT: We are relying on the major library version here. */
#define CRINIT_ELOS_LIBRARY "libelos.so.0"  ///< Elos shared library name
#define CRINIT_ELOS_DEPENDENCY "@elos"      ///< Elos filter dependency prefix

static bool crinitUseElos =
    CRINIT_CONFIG_DEFAULT_USE_ELOS;  ///< Specifies if we should use syslog calls instead of FILE streams.

#define ELOS_ID_INVALID 0  ///< Invalid elos event queue ID constant.

/**
 * Possible elos return values.
 */
typedef enum crinitSafuResultE_t {
    SAFU_RESULT_FAILED = -1,
    SAFU_RESULT_OK = 0,
    SAFU_RESULT_NOT_FOUND = 1,
} crinitSafuResultE_t;

/**
 * Type defition for elos event queue IDs.
 */
typedef uint32_t crinitElosEventQueueId_t;

/**
 * Elos session type.
 */
typedef struct crinitElosSession {
    int fd;          ///< Connection socket file descriptor
    bool connected;  ///< Connection state
} crinitElosSession_t;

/**
 * Elos event vector type.
 */
typedef struct crinitElosEventVector {
    size_t memorySize;      ///< Size of memory used
    size_t elementSize;     ///< Size of a single element
    uint32_t elementCount;  ///< Number of elements in the event vector
    void *data;             ///< Continous data block holding all elements
} crinitElosEventVector_t;

/**
 * Task that has unfulfilled filter dependencies.
 */
typedef struct crinitElosioFilterTask {
    crinitTask_t *task;          ///< The monitored task
    crinitList_t filterList;     ///< List unfulfilled filter dependencies
    pthread_mutex_t filterLock;  ///< Lock protecting the list of filters
    crinitList_t list;           ///< List handle for filter task list
} crinitElosioFilterTask_t;

/**
 * Definition of a single filter related to a task.
 */
typedef struct crinitElosioFilter {
    char *name;                             ///< Name of the filter
    char *filter;                           ///< The filter rule string
    crinitElosEventQueueId_t eventQueueId;  ///< ID of the elos event queue related to this filter
    crinitList_t list;                      ///< List handle for filter list
} crinitElosioFilter_t;

/**
 * Thread conext of the elosio main thread and elos vtable.
 */
static struct crinitElosEventThread {
    pthread_t threadId;            ///< Thread identifier
    char *elosServer;              ///< Elos server name or ip
    int elosPort;                  ///< Elos server port
    bool elosStarted;              ///< Wether or not an initial conenction to elos has been established
    crinitTaskDB_t *taskDb;        ///< Pointer to crinit task database
    crinitElosSession_t *session;  ///< Elos session handle
    crinitSafuResultE_t (*connect)(const char *, uint16_t,
                                   crinitElosSession_t **);  ///< Function pointer to the elosConnectTcpip function
    crinitSafuResultE_t (*getVersion)(crinitElosSession_t *,
                                      const char **);  ///< Function pointer to the elosGetVersion function
    crinitSafuResultE_t (*eventSubscribe)(
        crinitElosSession_t *, const char *[], size_t,
        crinitElosEventQueueId_t *);  ///< Function pointer to the elosEventSubscribe function
    crinitSafuResultE_t (*eventUnsubscribe)(
        crinitElosSession_t *, crinitElosEventQueueId_t);  ///< Function pointer to the elosEventUnsubscribe function
    crinitSafuResultE_t (*eventQueueRead)(
        crinitElosSession_t *, crinitElosEventQueueId_t,
        crinitElosEventVector_t **);                            ///< Function pointer to the elosEventQueueRead function
    void *(*eventVecGetLast)(const crinitElosEventVector_t *);  ///< Function pointer to the safuVecGetLast function
    void (*eventVectorDelete)(crinitElosEventVector_t *);  ///< Function pointer to the elosEventVectorDelete function
    crinitSafuResultE_t (*disconnect)(crinitElosSession_t *);  ///< Function pointer to the elosDisconnect function
} crinitTinfo;

/** List of tasks with elos filter dependencies **/
static crinitList_t crinitFilterTasks = CRINIT_LIST_INIT(crinitFilterTasks);

/** Mutex synchronizing elos filter task registration **/
static pthread_mutex_t crinitElosioFilterTaskLock = PTHREAD_MUTEX_INITIALIZER;

/** Mutex synchronizing elos connection **/
static pthread_mutex_t crinitElosioSessionLock = PTHREAD_MUTEX_INITIALIZER;

/**
 * Macro to simplify checking for a valid elos session.
 *
 * Will print an error message and return from the calling function with an error code if the
 * session pointer is either null or the conn.
 *
 * HINT: This uses a GNU extension of gcc to define a compound statement enclosed in parentheses.
 *
 * @param func      Elos function to be called.
 * @param err_msg   Error message to be returned on error.
 */
#define crinitElosioTryExec(func, err_msg, ...)                                                        \
    __extension__({                                                                                    \
        int res = SAFU_RESULT_OK;                                                                      \
                                                                                                       \
        if ((errno = pthread_mutex_lock(&crinitElosioSessionLock)) != 0) {                             \
            crinitErrnoPrint("Failed to lock elos session.");                                          \
            res = -1;                                                                                  \
        } else {                                                                                       \
            while (crinitTinfo.session == NULL || !crinitTinfo.session->connected) {                   \
                if (crinitTinfo.elosServer == NULL) {                                                  \
                    crinitErrPrint("Elos server configuration missing or not loaded yet.");            \
                    res = -1;                                                                          \
                    break;                                                                             \
                } else {                                                                               \
                    if (crinitTinfo.session != NULL) {                                                 \
                        crinitTinfo.disconnect(crinitTinfo.session);                                   \
                    }                                                                                  \
                                                                                                       \
                    res = crinitTinfo.connect(crinitTinfo.elosServer, crinitTinfo.elosPort,            \
                                              (crinitElosSession_t **)&crinitTinfo.session);           \
                    if (res != SAFU_RESULT_OK) {                                                       \
                        crinitErrPrint("Failed to connect to elosd on %s:%d.", crinitTinfo.elosServer, \
                                       crinitTinfo.elosPort);                                          \
                        usleep(500000);                                                                \
                    }                                                                                  \
                }                                                                                      \
            }                                                                                          \
                                                                                                       \
            if (res == SAFU_RESULT_OK) {                                                               \
                res = func(__VA_ARGS__);                                                               \
                if (res != SAFU_RESULT_OK) {                                                           \
                    crinitErrPrint(err_msg);                                                           \
                }                                                                                      \
                                                                                                       \
                if ((errno = pthread_mutex_unlock(&crinitElosioSessionLock)) != 0) {                   \
                    crinitErrnoPrint("Failed to unlock elos session.");                                \
                    res = -1;                                                                          \
                }                                                                                      \
            }                                                                                          \
        }                                                                                              \
                                                                                                       \
        res;                                                                                           \
    })

/**
 * Frees the heap allocated members of the filter.
 *
 * @param filter Filter to be destroyed.
 */
static void crinitElosioFilterDestroy(crinitElosioFilter_t *filter) {
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
static int crinitElosioFilterRegister(crinitElosioFilterTask_t *filterTask, crinitElosioFilter_t *filter) {
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
static int crinitElosioFilterUnregister(crinitElosioFilterTask_t *filterTask, crinitElosioFilter_t *filter) {
    if ((errno = pthread_mutex_lock(&filterTask->filterLock)) != 0) {
        crinitErrnoPrint("Failed to lock elos filter list.");
        return -1;
    }

    crinitListDelete(&filter->list);
    crinitElosioFilterDestroy(filter);

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
static int crinitElosioFilterListClear(crinitElosioFilterTask_t *filterTask) {
    crinitElosioFilter_t *cur, *temp;

    if ((errno = pthread_mutex_lock(&filterTask->filterLock)) != 0) {
        crinitErrnoPrint("Failed to lock elos filter list.");
        return -1;
    }

    crinitListForEachEntrySafe(cur, temp, &filterTask->filterList, list) {
        crinitListDelete(&cur->list);
        crinitElosioFilterDestroy(cur);
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
static int crinitElosioFilterTaskDestroy(crinitElosioFilterTask_t *filterTask) {
    int res = crinitElosioFilterListClear(filterTask);
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
static int crinitElosioFilterTaskListClear() {
    int res = 0;
    crinitElosioFilterTask_t *cur, *temp;

    /* Insert into list of filters of filter task */
    if ((errno = pthread_mutex_lock(&crinitElosioFilterTaskLock)) != 0) {
        crinitErrnoPrint("Failed to lock elos filter task list.");
        return -1;
    }

    crinitListForEachEntrySafe(cur, temp, &crinitFilterTasks, list) {
        crinitListDelete(&cur->list);
        if ((res = crinitElosioFilterTaskDestroy(cur)) != 0) {
            crinitErrnoPrint("Failed to lock elos filter task list.");
            res = -1;
            break;
        }
    }

    if ((errno = pthread_mutex_unlock(&crinitElosioFilterTaskLock)) != 0) {
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
static int crinitElosioFilterFromEnvSet(const crinitTask_t *task, const char *name, crinitElosioFilter_t **filter) {
    int res = -1;
    crinitElosioFilter_t *temp;
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
static inline int crinitElosioFilterSubscribe(crinitElosioFilter_t *filter) {
    crinitDbgInfoPrint("Try to subscribe with filter %s: %s\n", filter->name, filter->filter);
    return crinitElosioTryExec(crinitTinfo.eventSubscribe, "Failed to subscribe with filter.", crinitTinfo.session,
                               (const char **)&filter->filter, 1, &filter->eventQueueId);
}

/**
 * Subscribes all filters currently registered with elosio.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
static int crinitElosioFilterListSubscribe(void) {
    int res = 0;
    crinitElosioFilter_t *filter;
    crinitElosioFilterTask_t *filterTask;

    if ((errno = pthread_mutex_lock(&crinitElosioFilterTaskLock)) != 0) {
        crinitErrnoPrint("Failed to lock elos filter task list.");
        return -1;
    }

    crinitListForEachEntry(filterTask, &crinitFilterTasks, list) {
        crinitListForEachEntry(filter, &filterTask->filterList, list) {
            if ((res = crinitElosioFilterSubscribe(filter)) != 0) {
                crinitErrPrint("Failed to subscribe filter.");
                goto err;
            }
        }
    }

err:
    if ((errno = pthread_mutex_unlock(&crinitElosioFilterTaskLock)) != 0) {
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
static inline int crinitElosioFilterUnsubscribe(crinitElosioFilter_t *filter) {
    return crinitElosioTryExec(crinitTinfo.eventUnsubscribe, "Failed to unsubscribe filter.", crinitTinfo.session,
                               filter->eventQueueId);
}

/**
 * Extracts all filter defines for each task and creates a local
 * elos filter handle, which will be used to subscribe for this filter.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
static int crinitElosioFilterListUnsubscribe(void) {
    int res = 0;
    crinitElosioFilter_t *filter;
    crinitElosioFilterTask_t *filterTask;

    if ((errno = pthread_mutex_lock(&crinitElosioFilterTaskLock)) != 0) {
        crinitErrnoPrint("Failed to lock elos filter task list.");
        return -1;
    }

    crinitListForEachEntry(filterTask, &crinitFilterTasks, list) {
        crinitListForEachEntry(filter, &filterTask->filterList, list) {
            if ((res = crinitElosioFilterUnsubscribe(filter)) != 0) {
                crinitErrPrint("Failed to subscribe filter.");
                goto err;
            }
        }
    }

err:
    if ((errno = pthread_mutex_unlock(&crinitElosioFilterTaskLock)) != 0) {
        crinitErrnoPrint("Failed to unlock elos filter list.");
        res = -1;
    }

    return res;
}

int crinitElosioTaskAdded(crinitTask_t *task) {
    int res = 0;
    crinitTaskDep_t *dep;
    crinitElosioFilter_t *filter = NULL;
    crinitElosioFilterTask_t *filterTask = NULL;

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

            if ((errno = pthread_mutex_lock(&crinitElosioFilterTaskLock)) != 0) {
                crinitErrnoPrint("Failed to lock elos filter task list.");
                res = -1;
                break;
            }

            crinitListAppend(&crinitFilterTasks, &filterTask->list);

            if ((errno = pthread_mutex_unlock(&crinitElosioFilterTaskLock)) != 0) {
                crinitErrnoPrint("Failed to unlock elos filter task list.");
                res = -1;
                break;
            }
        }

        crinitDbgInfoPrint("Searching for filter for dependency %s:%s.", dep->name, dep->event);
        if ((res = crinitElosioFilterFromEnvSet(task, dep->event, &filter)) != 0) {
            crinitErrPrint("Failed to find filter for dependency %s:%s.", dep->name, dep->event);
            break;
        }

        if ((res = crinitElosioFilterRegister(filterTask, filter)) != 0) {
            crinitErrPrint("Failed to register filter for dependency %s:%s.", dep->name, dep->event);
            crinitElosioFilterDestroy(filter);
            break;
        }

        /* Subscribing the filter might fail if elos is not started yet */
        if (crinitTinfo.elosStarted && (res = crinitElosioFilterSubscribe(filter)) != 0) {
            crinitErrPrint("Failed to subscribe filter for dependency %s:%s.", dep->name, dep->event);
            break;
        }
    }

    return res;
}

/**
 * Disconnect from elos daemon.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
static inline int crinitElosioDisconnect(void) {
    int res = SAFU_RESULT_OK;

    if (crinitTinfo.session != NULL) {
        if ((errno = pthread_mutex_lock(&crinitElosioSessionLock)) != 0) {
            crinitErrnoPrint("Failed to lock elos session.");
            return SAFU_RESULT_FAILED;
        }

        res = crinitTinfo.disconnect(crinitTinfo.session);
        if (res != SAFU_RESULT_OK) {
            crinitErrPrint("Failed to disconnect from elosd.");
        }

        if ((errno = pthread_mutex_unlock(&crinitElosioSessionLock)) != 0) {
            crinitErrnoPrint("Failed to unlock elos session.");
            res = SAFU_RESULT_FAILED;
        }
    }

    return res;
}

static void *crinitElosioEventListener(void *arg) {
    int err = -1;
    char *elosServer;
    int elosPort;
    const char *version;
    crinitElosioFilter_t *filter, *tempFilter;
    crinitElosioFilterTask_t *filterTask;
    crinitElosEventVector_t *eventVector = NULL;

    struct crinitElosEventThread *tinfo = arg;

    tinfo->elosStarted = true;

    crinitDbgInfoPrint("Loading elos connection parameters.");
    if (crinitGlobOptGet(CRINIT_GLOBOPT_ELOS_SERVER, &elosServer) == -1) {
        crinitErrPrint("Could not recall elos server ip from global options.");
        goto err;
    } else {
        if (tinfo->elosServer != NULL) {
            free(tinfo->elosServer);
        }

        tinfo->elosServer = elosServer;
    }

    if (crinitGlobOptGet(CRINIT_GLOBOPT_ELOS_PORT, &elosPort) == -1) {
        crinitErrPrint("Could not recall elos server port from global options.");
        goto err_options;
    } else {
        tinfo->elosPort = elosPort;
    }

    crinitDbgInfoPrint("Got elos connection parameters %s:%d.", tinfo->elosServer, tinfo->elosPort);

    err = crinitElosioTryExec(tinfo->getVersion, "Failed to request elos version.", tinfo->session, &version);
    if (err == SAFU_RESULT_OK) {
        crinitInfoPrint("Connected to elosd running version: %s", version);
    }

    if ((err = crinitElosioFilterListSubscribe()) != 0) {
        crinitErrnoPrint("Failed to subscribe already registered elos filters.");
        goto err_connection;
    }

    while (1) {
        crinitListForEachEntry(filterTask, &crinitFilterTasks, list) {
            crinitListForEachEntrySafe(filter, tempFilter, &filterTask->filterList, list) {
                err = crinitElosioTryExec(tinfo->eventQueueRead, "Failed to read elos event queue.", tinfo->session,
                                          filter->eventQueueId, &eventVector);
                if (err == SAFU_RESULT_OK && eventVector && eventVector->elementCount > 0) {
                    const crinitTaskDep_t taskDep = {
                        .name = CRINIT_ELOS_DEPENDENCY,
                        .event = filter->name,
                    };

                    if ((err = crinitTaskDBFulfillDep(tinfo->taskDb, &taskDep, filterTask->task)) != 0) {
                        crinitErrnoPrint("Failed to fulfill dependency %s:%s.", taskDep.name, taskDep.event);
                        goto err_session;
                    }

                    if ((err = crinitElosioFilterUnregister(filterTask, filter)) != 0) {
                        crinitErrnoPrint("Failed to remove filter from dependency list.");
                        goto err_session;
                    }
                }
            }
        }

        usleep(500000);
    }

err_session:
    if ((err = crinitElosioFilterListUnsubscribe()) != 0) {
        crinitErrnoPrint("Failed to unsubscribe elos filters.");
    }

    if ((err = crinitElosioFilterTaskListClear()) != 0) {
        crinitErrnoPrint("Failed to clear filter tasks.");
    }

err_connection:
    crinitElosioDisconnect();

err_options:
    free(tinfo->elosServer);

err:
    return NULL;
}

/**
 * Fetches a single symbol from the elos client shared library.
 *
 * @param lp Pointer to the elos shared library
 * @param symbolName Name of the symbol to be fetched
 * @param symbol Function pointer to be assigned
 * @return Returns 1 on success, 0 otherwise.
 */
static inline int crinitElosioFetchElosSymbol(void *lp, const char *symbolName, void **symbol) {
    char *err;

    /* Clear any existing error */
    dlerror();

    *(void **)(symbol) = dlsym(lp, symbolName);
    if ((err = dlerror()) != NULL) {
        crinitErrPrint("Failed to load '%s' from %s: %s.", symbolName, CRINIT_ELOS_LIBRARY, err);
        dlclose(lp);
        return 0;
    }

    return 1;
}

/**
 * Initializes the vtable managed within the elosio thread context.
 *
 * @param taskDb Pointer to the crinit task database
 * @param tinfo Elosio thread context
 * @return Returns 0 on success, -1 otherwise.
 */
static int crinitElosioInitThreadContext(crinitTaskDB_t *taskDb, struct crinitElosEventThread *tinfo) {
    int res = 0;
    void *lp;

    tinfo->taskDb = taskDb;
    tinfo->elosPort = CRINIT_CONFIG_DEFAULT_ELOS_PORT;
    tinfo->elosServer = strdup(CRINIT_CONFIG_DEFAULT_ELOS_SERVER);

    lp = dlopen(CRINIT_ELOS_LIBRARY, RTLD_NOW | RTLD_LOCAL);
    if (!lp) {
        crinitErrPrint("Failed to load dynamic library %s: %s.", CRINIT_ELOS_LIBRARY, dlerror());
        return -1;
    }

    res = crinitElosioFetchElosSymbol(lp, "elosConnectTcpip", (void **)&tinfo->connect) &&
          crinitElosioFetchElosSymbol(lp, "elosGetVersion", (void **)&tinfo->getVersion) &&
          crinitElosioFetchElosSymbol(lp, "elosEventSubscribe", (void **)&tinfo->eventSubscribe) &&
          crinitElosioFetchElosSymbol(lp, "elosEventUnsubscribe", (void **)&tinfo->eventUnsubscribe) &&
          crinitElosioFetchElosSymbol(lp, "elosEventQueueRead", (void **)&tinfo->eventQueueRead) &&
          crinitElosioFetchElosSymbol(lp, "safuVecGetLast", (void **)&tinfo->eventVecGetLast) &&
          crinitElosioFetchElosSymbol(lp, "elosEventVectorDelete", (void **)&tinfo->eventVectorDelete) &&
          crinitElosioFetchElosSymbol(lp, "elosDisconnect", (void **)&tinfo->disconnect);

    return (res == 0) ? -1 : 0;
}

int crinitElosioActivate(crinitTaskDB_t *taskDb, bool sl) {
    int res;

    if (sl && !crinitUseElos) {
        if (crinitElosioInitThreadContext(taskDb, &crinitTinfo) != 0) {
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
        res = pthread_create(&crinitTinfo.threadId, &attrs, crinitElosioEventListener, (void *)&crinitTinfo);
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
    } else if (!sl && crinitUseElos) {
        // TODO: Kill thread.
        crinitUseElos = sl;
    }

    return 0;
}
