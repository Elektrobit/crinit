// SPDX-License-Identifier: MIT
/**
 * @file taskdb.h
 * @brief Header defining the central Task Data Structure and related types/functions.
 */
#ifndef __TASKDB_H__
#define __TASKDB_H__

#include <pthread.h>
#include <stddef.h>

#include "task.h"

#define CRINIT_MONITOR_DEP_NAME "@crinitmon"  ///< Special dependency name to depend on monitor events (not yet impl.).
#define CRINIT_PROVIDE_DEP_NAME "@provided"   ///< Special dependency name to depend on provided system features.
#define CRINIT_TASKDB_INITIAL_SIZE 256        ///< Default initial size of taskSet and inclSet within an crinitTaskDB_t.

/**
 * Type to describe wheter the spawn thread launches start or stop commands
 */
typedef enum crinitDispatchThreadMode_t {
    CRINIT_DISPATCH_THREAD_MODE_START,
    CRINIT_DISPATCH_THREAD_MODE_STOP
} crinitDispatchThreadMode_t;

/**
 * Type to store a task database.
 */
typedef struct crinitTaskDB_t {
    crinitTask_t *taskSet;  ///< Dynamic array of tasks, corresponds to task configs specified in the series config.
    size_t taskSetSize;     ///< Current maximum size of the task array.
    size_t taskSetItems;    ///< Number of elements in the task array.

    /** Pointer specifying a function for spawning ready tasks, used by crinitTaskDBSpawnReady() **/
    int (*spawnFunc)(struct crinitTaskDB_t *ctx, const crinitTask_t *, crinitDispatchThreadMode_t mode);

    bool
        spawnInhibit;  ///< Specifies if process spawning is currently inhibited, respected by crinitTaskDBSpawnReady().

    pthread_mutex_t lock;    ///< Mutex to lock the TaskDB, shall be used for any operations on the data structure if
                             ///< multiple threads are involved.
    pthread_cond_t changed;  ///< Condition variable to be signalled if taskSet or spawnInhibit is changed.
} crinitTaskDB_t;

/**
 * Iterate over all tasks in a task database
 *
 * @param taskDb  Pointer to a task database to traverse.
 * @param task    Pointer to a single task entry.
 */
#define crinitTaskDbForEach(taskDb, task) \
    for ((task) = (taskDb)->taskSet; (task) != (taskDb)->taskSet + (taskDb)->taskSetItems; (task)++)

/**
 * Insert a task into a task database.
 *
 * Will store a copy of \a t in the crinitTaskDB_t::taskSet of \a ctx. crinitTaskDB_t::taskSetItems will be incremented
 * and if crinitTaskDB_t::taskSetSize is not sufficient, the set will be grown. If \a overwrite is true, a task with the
 * same name in the set will be overwritten. If it is false, an existing task with the same name will cause an error. If
 * the task has been successfully inserted, the function will signal crinitTaskDB_t::changed. The function uses
 * crinitTaskDB_t::lock for synchronization and is thread-safe.
 *
 * @param ctx        The crinitTaskDB_t context, into which task should be inserted.
 * @param t          The task to be inserted.
 * @param overwrite  Overwrite a colliding task with the same name (true) or return an error (false).
 *
 * @return 0 on success, -1 otherwise
 */
int crinitTaskDBInsert(crinitTaskDB_t *ctx, const crinitTask_t *t, bool overwrite);
/**
 * Insert a task into a task database, overwriting a task with the same name if it exists.
 *
 * For explanation see crinitTaskDBInsert() with parameter \a overwrite set to true.
 */
#define crinitTaskDBUpdate(ctx, t) crinitTaskDBInsert(ctx, t, true)

/**
 * Fulfill a dependency for all tasks inside a task database.
 *
 * Will search \a ctx for tasks containing a dependency equal to \a dep (i.e. specifying the same name and event,
 * according to strcmp()) and, if found, remove the dependency from crinitTask_t::deps. Will signal
 * crinitTaskDB_t::changed on successful completion. The function uses crinitTaskDB_t::lock for synchronization and is
 * thread-safe.
 *
 * @param ctx  The crinitTaskDB_t context in which to fulfill the dependency.
 * @param dep  The dependency to be fulfilled.
 * @param target The targeted task or NULL.
 *
 * @return 0 on success, -1 otherwise
 */
int crinitTaskDBFulfillDep(crinitTaskDB_t *ctx, const crinitTaskDep_t *dep, const crinitTask_t *target);
/**
 * Fulfill feature dependencies implemented by a provider task.
 *
 * Will search \a ctx for tasks containing feature dependencies PROVIDE-ed by \a provider given its new state
 * \a newState and, if found, remove the dependency from crinitTask_t::dep by using crinitTaskDBFulfillDep().
 * Synchronization and signalling remains the same as with a direct call to crinitTaskDBFulfillDep().
 *
 * @param ctx       The crinitTaskDB_t context in which to fulfill the feature dependency.
 * @param provider  The crinitTask_t providing the feature(s).
 * @param newState  The crinitTaskState_t which has been newly reached by \a provider.
 *
 * @return 0 on success, -1 otherwise
 */
int crinitTaskDBProvideFeature(crinitTaskDB_t *ctx, const crinitTask_t *provider, crinitTaskState_t newState);
/**
 * Fulfill feature dependencies implemented by a provider task (searched for by name).
 *
 * Will search \a ctx for the provider task referenced by \a taskName and then call crinitTaskDBProvideFeature() on it
 * (see there for details).
 *
 * @param ctx       The crinitTaskDB_t context in which to fulfill the feature dependency.
 * @param taskName  The name of the task providing the feature(s).
 * @param newState  The crinitTaskState_t which has been newly reached by the provider task.
 *
 * @return 0 on success, -1 otherwise
 */
int crinitTaskDBProvideFeatureByTaskName(crinitTaskDB_t *ctx, const char *taskName, crinitTaskState_t newState);
/**
 * Add a dependency to a specific task inside a task database.
 *
 * Will search \a ctx for a task with name \a taskName and add \a dep to its crinitTask_t::deps and adjust
 * crinitTask_t::depsSize.
 *
 * @param ctx       The crinitTaskDb context to work on.
 * @param dep       The dependency to be added.
 * @param taskName  The name of the relevant task.
 *
 * @return 0 on success, -1 otherweise
 */
int crinitTaskDBAddDepToTask(crinitTaskDB_t *ctx, const crinitTaskDep_t *dep, const char *taskName);
/**
 * Remove a dependency from a specific task inside a task database.
 *
 * Will search \a ctx for a task with name \a taskName and remove a dependency equal to \a dep from its
 * crinitTask_t::deps and adjust crinitTask_t::depsSize, if such a dependency is present. The equality condition
 * between two crinitTaskDep_t instances is the same as in crinitTaskDBFulfillDep(), i.e. their contents are
 * lexicographically equal.
 *
 * @param ctx       The crinitTaskDb context to work on.
 * @param dep       The dependency to be removed.
 * @param taskName  The name of the relevant task.
 *
 * @return 0 on success, -1 otherweise
 */
int crinitTaskDBRemoveDepFromTask(crinitTaskDB_t *ctx, const crinitTaskDep_t *dep, const char *taskName);

/**
 * Set the crinitTaskState_t of a task in a task database
 *
 * Will search \a ctx for an crinitTask_t with crinitTask_t::name lexicographically equal to \a taskName and set its
 * crinitTask_t::state to \a s. If such a task does not exist in \a ctx, an error is returned. If \a s equals
 * #CRINIT_TASK_STATE_FAILED, crinitTask_t::failCount will be incremented by 1. If \a s equals #CRINIT_TASK_STATE_DONE,
 * crinitTask_t::failCount will be reset to 0. The function uses crinitTaskDB_t::lock for synchronization and is
 * thread-safe.
 *
 * @param ctx       The crinitTaskDB_t context in which the task is held.
 * @param s         The task's new state.
 * @param taskName  The task's name.
 *
 * @return 0 on success, -1 otherwise.
 */
int crinitTaskDBSetTaskState(crinitTaskDB_t *ctx, crinitTaskState_t s, const char *taskName);
/**
 * Get the crinitTaskState_t of a task in a task database
 *
 * Will search \a ctx for an crinitTask_t with crinitTask_t::name lexicographically equal to \a taskName and write its
 * crinitTask_t::state to \a s. If such a task does not exist in \a ctx, an error is returned. The function uses
 * crinitTaskDB_t::lock for synchronization and is thread-safe.
 *
 * @param ctx       The crinitTaskDB_t context in which the task is held.
 * @param s         Pointer to store the returned crinitTaskState_t.
 * @param taskName  The task's name.
 *
 * @return 0 on success, -1 otherwise.
 */
int crinitTaskDBGetTaskState(crinitTaskDB_t *ctx, crinitTaskState_t *s, const char *taskName);

/**
 * Set the PID a task in a task database
 *
 * Will search \a ctx for an crinitTask_t with crinitTask_t::name lexicographically equal to \a taskName and set its
 * crinitTask_t::pid to \a pid. If such a task does not exit in \a ctx, an error is returned. The function uses
 * crinitTaskDB_t::lock for synchronization and is thread-safe.
 *
 * @param ctx       The crinitTaskDB_t context in which the task is held.
 * @param pid       The task's new PID.
 * @param taskName  The task's name.
 *
 * @return 0 on success, -1 otherwise.
 */
int crinitTaskDBSetTaskPID(crinitTaskDB_t *ctx, pid_t pid, const char *taskName);
/**
 * Get the PID of a task in a task database
 *
 * Will search \a ctx for an crinitTask_t with crinitTask_t::name lexicographically equal to \a taskName and write its
 * PID to \a pid. If such a task does not exit in \a ctx, an error is returned. If the task does not currently have a
 * running process, \a pid will be -1 but the function will indicate success. The function uses crinitTaskDB_t::lock for
 * synchronization and is thread-safe.
 *
 * @param ctx       The crinitTaskDB_t context in which the task is held.
 * @param pid       Pointer to store the returned PID.
 * @param taskName  The task's name.
 *
 * @return 0 on success, -1 otherwise.
 */
int crinitTaskDBGetTaskPID(crinitTaskDB_t *ctx, pid_t *pid, const char *taskName);

/**
 * Get the crinitTaskState_t and the PID of a task in a task database
 *
 * Will search \a ctx for an crinitTask_t with crinitTask_t::name lexicographically equal to \a taskName and write its
 * crinitTask_t::state to \a s and its PID to \a pid. If such a task does not exist in \a ctx, an error is returned. If
 * the task does not currently have a running process, \a pid will be -1 but the function will indicate success. The
 * function uses crinitTaskDB_t::lock for synchronization and is thread-safe.
 *
 * @param ctx       The crinitTaskDB_t context in which the task is held.
 * @param s         Pointer to store the returned crinitTaskState_t.
 * @param pid       Pointer to store the returned PID.
 * @param taskName  The task's name.
 *
 * @return 0 on success, -1 otherwise.
 */
int crinitTaskDBGetTaskStateAndPID(crinitTaskDB_t *ctx, crinitTaskState_t *s, pid_t *pid, const char *taskName);

/**
 * Provide direct thread-safe access to a task within a task database
 *
 * If a task with the given name is found within the task database context, the calling thread will hold an exclusive
 * lock on the context and get a reference to the task in question via return value. After the calling thread has
 * finished its operations on the global option storage, it must release the lock using crinitTaskDBRemit().
 *
 * If the function returns an error (`NULL`), no database lock is acquired.
 *
 * As the lock is held on the whole task database, operations in the critical section between crinitTaskDBBorrowTask
 * and crinitTaskDBRemit() must be kept short to avoid performance issues.
 *
 * @param ctx       The TaskDB containing the task to borrow.
 * @param taskName  The name of the task to borrow.
 *
 * @return  A pointer to the task with \a taskName within the task database context on success, NULL on any error.
 */
crinitTask_t *crinitTaskDBBorrowTask(crinitTaskDB_t *ctx, const char *taskName);
/**
 * Release the lock on the task database acquired via crinitTaskDBBorrowTask(). The borrowed task reference may not be
 * used anymore.
 *
 * @param ctx  The TaskDB to release the lock from.
 *
 * @return  0 on success, -1 if the lock could not be released.
 */
int crinitTaskDBRemit(crinitTaskDB_t *ctx);

/**
 * Run crinitTaskDB_t::spawnFunc for each startable task in a task database.
 *
 * A task is startable if and only if it has no remaining crinitTask_t::deps and it has either not been started before
 * according to crinitTask_t::state or it should be respawned. A task should be respawned if and only if
 * crinitTask_t::opts contains the flag #CRINIT_TASK_OPT_RESPAWN and either crinitTask_t::maxRetries is -1 or
 * crinitTask_t::failCount is less than crinitTask_t::maxRetries. The function uses crinitTaskDB_t::lock for
 * synchronization and is thread-safe.
 *
 * If crinitTaskDB::spawnInhibit is true, no tasks are considered startable and this function will return successfully
 * without starting anything.
 *
 * @param ctx  The TaskDB context from which tasks will be started.
 * @param mode Distinguishes between start and stop commands
 *
 * @return 0 on success, -1 otherwise
 */
int crinitTaskDBSpawnReady(crinitTaskDB_t *ctx, crinitDispatchThreadMode_t mode);
/**
 * Inhibit or un-inhibit spawning of processes by setting crinitTaskDB_t::spawnInhibit.
 *
 * The function uses crinitTaskDB_t::lock for synchronization and is thread-safe. It will also signal
 * crinitTaskDB_t::changed if crinitTaskDB_t::spawnInhibit was changed to false.
 *
 * @param ctx  The TaskDB context in which to set the variable.
 * @param inh  The value which crinitTaskDB_t:spawnInhibit shall be set to.
 *
 * @return 0 on success, -1 otherwise
 */
int crinitTaskDBSetSpawnInhibit(crinitTaskDB_t *ctx, bool inh);

/**
 *  Initialize the internals of an crinitTaskDB_t with a specified initial size for crinitTaskDB_t::taskSet.
 *
 *  If the initialized TaskDB is no longer needed the internally held dynamic memory should be freed using
 *  crinitTaskDBDestroy().
 *
 *  @param ctx          The crinitTaskDB_t whose internal members should be initialized.
 *  @param spawnFunc    Pointer to the task spawn function to be used by crinitTaskDBSpawnReady()
 *  @param initialSize  Initial size of crinitTaskDB_t::taskSet for \a ctx.
 *
 *  @return 0 on success, -1 otherwise
 */
int crinitTaskDBInitWithSize(crinitTaskDB_t *ctx, int (*spawnFunc)(crinitTaskDB_t *ctx, const crinitTask_t *, crinitDispatchThreadMode_t mode),
                             size_t initialSize);
/**
 * Initialize the internals of an crinitTaskDB_t with the default initial size of CRINIT_TASKDB_INITIAL_SIZE.
 *
 * See crinitTaskDBInitWithSize().
 */
#define crinitTaskDBInit(ctx, spawnFunc) crinitTaskDBInitWithSize(ctx, spawnFunc, CRINIT_TASKDB_INITIAL_SIZE);

/**
 * Free memory allocated for crinitTaskDB_t members by functions like crinitTaskDBInit or crinitTaskDBInsert().
 *
 * Afterwards \a ctx may not be used anymore until another call to crinitTaskDBInit() or crinitTaskDBInitWithSize().
 *
 * @param ctx The crinitTaskDB_t context to be destroyed.
 *
 * @return 0 on success, -1 on error
 */
int crinitTaskDBDestroy(crinitTaskDB_t *ctx);

/**
 * Export the list of task names currently in the task database.
 *
 * The function allocates an array of strings as \a tasks and returns the number of array elements in \a numTasks.
 * Each entry in the \a tasks array will be allocated separately and needs to be freed by the caller.
 *
 * @param ctx       The TaskDB context from which to get the list of task names.
 * @param tasks     The return pointer for the array of task names.
 * @param numTasks  The return pointer for the number of array entries.
 *
 * @return 0 on success, -1 on error
 */
int crinitTaskDBExportTaskNamesToArray(crinitTaskDB_t *ctx, char **tasks[], size_t *numTasks);

#endif /* __TASKDB_H__ */
