/**
 * @file taskdb.h
 * @brief Header defining the central Task Data Structure and related types/functions.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __TASKDB_H__
#define __TASKDB_H__

#include <pthread.h>
#include <stddef.h>

#include "task.h"

#define EBCL_MONITOR_DEP_NAME "@ebclmon"   ///< Special dependency name to depend on monitor events (not yet impl.).
#define EBCL_PROVIDE_DEP_NAME "@provided"  ///< Special dependency name to depend on provided system features.
#define EBCL_TASKDB_INITIAL_SIZE 256       ///< Default initial size of taskSet and inclSet within an ebcl_TaskDB_t.

/**
 * Type to store a task database.
 */
typedef struct ebcl_TaskDB_t {
    ebcl_Task_t *taskSet;  ///< Dynamic array of tasks, corresponds to task configs specified in the series config.
    size_t taskSetSize;    ///< Current maximum size of the task array.
    size_t taskSetItems;   ///< Number of elements in the task array.

    /** Pointer specifying a function for spawning ready tasks, used by EBCL_taskDBSpawnReady() **/
    int (*spawnFunc)(struct ebcl_TaskDB_t *ctx, const ebcl_Task_t *);

    bool spawnInhibit;  ///< Specifies if process spawning is currently inhibited, respected by EBCL_taskDBSpawnReady().

    pthread_mutex_t lock;    ///< Mutex to lock the TaskDB, shall be used for any operations on the data structure if
                             ///< multiple threads are involved.
    pthread_cond_t changed;  ///< Condition variable to be signalled if taskSet or spawnInhibit is changed.
} ebcl_TaskDB_t;

/**
 * Insert a task into a task database.
 *
 * Will store a copy of \a t in the ebcl_TaskDB_t::taskSet of \a ctx. ebcl_TaskDB_t::taskSetItems will be incremented
 * and if ebcl_TaskDB_t::taskSetSize is not sufficient, the set will be grown. If \a overwrite is true, a task with the
 * same name in the set will be overwritten. If it is false, an existing task with the same name will cause an error. If
 * the task has been successfully inserted, the function will signal ebcl_TaskDB_t::changed. The function uses
 * ebcl_TaskDB_t::lock for synchronization and is thread-safe.
 *
 * @param ctx        The ebcl_TaskDB_t context, into which task should be inserted.
 * @param t          The task to be inserted.
 * @param overwrite  Overwrite a colliding task with the same name (true) or return an error (false).
 *
 * @return 0 on success, -1 otherwise
 */
int EBCL_taskDBInsert(ebcl_TaskDB_t *ctx, const ebcl_Task_t *t, bool overwrite);
/**
 * Insert a task into a task database, overwriting a task with the same name if it exists.
 *
 * For explanation see EBCL_taskDBInsert() with parameter \a overwrite set to true.
 */
#define EBCL_taskDBUpdate(ctx, t) EBCL_taskDBInsert(ctx, t, true)

/**
 * Fulfill a dependency for all tasks inside a task database.
 *
 * Will search \a ctx for tasks containing a dependency equal to \a dep (i.e. specifying the same name and event,
 * according to strcmp()) and, if found, remove the dependency from ebcl_Task_t::deps. Will signal
 * ebcl_TaskDB_t::changed on successful completion. The function uses ebcl_TaskDB_t::lock for synchronization and is
 * thread-safe.
 *
 * @param ctx  The ebcl_TaskDB_t context in which to fulfill the dependency.
 * @param dep  The dependency to be fulfilled.
 *
 * @return 0 on success, -1 otherwise
 */
int EBCL_taskDBFulfillDep(ebcl_TaskDB_t *ctx, const ebcl_TaskDep_t *dep);
/**
 * Fulfill feature dependencies implemented by a provider task.
 *
 * Will search \a ctx for tasks containing feature dependencies PROVIDE-ed by \a provider given its new state
 * \a newState and, if found, remove the dependency from ebcl_Task_t::dep by using EBCL_taskDBFulfillDep().
 * Synchronization and signalling remains the same as with a direct call to EBCL_taskDBFulfillDep().
 *
 * @param ctx       The ebcl_TaskDB_t context in which to fulfill the feature dependency.
 * @param provider  The ebcl_Task_t providing the feature(s).
 * @param newState  The ebcl_TaskState_t which has been newly reached by \a provider.
 *
 * @return 0 on success, -1 otherwise
 */
int EBCL_taskDBProvideFeature(ebcl_TaskDB_t *ctx, const ebcl_Task_t *provider, ebcl_TaskState_t newState);
/**
 * Fulfill feature dependencies implemented by a provider task (searched for by name).
 *
 * Will search \a ctx for the provider task referenced by \a taskName and then call EBCL_taskDBProvideFeature() on it
 * (see there for details).
 *
 * @param ctx       The ebcl_TaskDB_t context in which to fulfill the feature dependency.
 * @param taskName  The name of the task providing the feature(s).
 * @param newState  The ebcl_TaskState_t which has been newly reached by the provider task.
 *
 * @return 0 on success, -1 otherwise
 */
int EBCL_taskDBProvideFeatureByTaskName(ebcl_TaskDB_t *ctx, const char *taskName, ebcl_TaskState_t newState);
/**
 * Add a dependency to a specific task inside a task database.
 *
 * Will search \a ctx for a task with name \a taskName and add \a dep to its ebcl_Task_t::deps and adjust
 * ebcl_Task_t::depsSize.
 *
 * @param ctx       The ebcl_taskDb context to work on.
 * @param dep       The dependency to be added.
 * @param taskName  The name of the relevant task.
 *
 * @return 0 on success, -1 otherweise
 */
int EBCL_taskDBAddDepToTask(ebcl_TaskDB_t *ctx, const ebcl_TaskDep_t *dep, const char *taskName);
/**
 * Remove a dependency from a specific task inside a task database.
 *
 * Will search \a ctx for a task with name \a taskName and remove a dependency equal to \a dep from its
 * ebcl_Task_t::deps and adjust ebcl_Task_t::depsSize, if such a dependency is present. The equality condition
 * between two ebcl_TaskDep_t instances is the same as in EBCL_taskDBFulfillDep(), i.e. their contents are
 * lexicographically equal.
 *
 * @param ctx       The ebcl_taskDb context to work on.
 * @param dep       The dependency to be removed.
 * @param taskName  The name of the relevant task.
 *
 * @return 0 on success, -1 otherweise
 */
int EBCL_taskDBRemoveDepFromTask(ebcl_TaskDB_t *ctx, const ebcl_TaskDep_t *dep, const char *taskName);

/**
 * Set the ebcl_TaskState_t of a task in a task database
 *
 * Will search \a ctx for an ebcl_Task_t with ebcl_Task_t::name lexicographically equal to \a taskName and set its
 * ebcl_Task_t::state to \a s. If such a task does not exist in \a ctx, an error is returned. If \a s equals
 * #EBCL_TASK_STATE_FAILED, ebcl_Task_t::failCount will be incremented by 1. If \a s equals #EBCL_TASK_STATE_DONE,
 * ebcl_Task_t::failCount will be reset to 0. The function uses ebcl_TaskDB_t::lock for synchronization and is
 * thread-safe.
 *
 * @param ctx       The ebcl_TaskDB_t context in which the task is held.
 * @param s         The task's new state.
 * @param taskName  The task's name.
 *
 * @return 0 on success, -1 otherwise.
 */
int EBCL_taskDBSetTaskState(ebcl_TaskDB_t *ctx, ebcl_TaskState_t s, const char *taskName);
/**
 * Get the ebcl_TaskState_t of a task in a task database
 *
 * Will search \a ctx for an ebcl_Task_t with ebcl_Task_t::name lexicographically equal to \a taskName and write its
 * ebcl_Task_t::state to \a s. If such a task does not exist in \a ctx, an error is returned. The function uses
 * ebcl_TaskDB_t::lock for synchronization and is thread-safe.
 *
 * @param ctx       The ebcl_TaskDB_t context in which the task is held.
 * @param s         Pointer to store the returned ebcl_TaskState_t.
 * @param taskName  The task's name.
 *
 * @return 0 on success, -1 otherwise.
 */
int EBCL_taskDBGetTaskState(ebcl_TaskDB_t *ctx, ebcl_TaskState_t *s, const char *taskName);

/**
 * Set the PID a task in a task database
 *
 * Will search \a ctx for an ebcl_Task_t with ebcl_Task_t::name lexicographically equal to \a taskName and set its
 * ebcl_Task_t::pid to \a pid. If such a task does not exit in \a ctx, an error is returned. The function uses
 * ebcl_TaskDB_t::lock for synchronization and is thread-safe.
 *
 * @param ctx       The ebcl_TaskDB_t context in which the task is held.
 * @param pid       The task's new PID.
 * @param taskName  The task's name.
 *
 * @return 0 on success, -1 otherwise.
 */
int EBCL_taskDBSetTaskPID(ebcl_TaskDB_t *ctx, pid_t pid, const char *taskName);
/**
 * Get the PID of a task in a task database
 *
 * Will search \a ctx for an ebcl_Task_t with ebcl_Task_t::name lexicographically equal to \a taskName and write its
 * PID to \a pid. If such a task does not exit in \a ctx, an error is returned. If the task does not currently have a
 * running process, \a pid will be -1 but the function will indicate success. The function uses ebcl_TaskDB_t::lock for
 * synchronization and is thread-safe.
 *
 * @param ctx       The ebcl_TaskDB_t context in which the task is held.
 * @param pid       Pointer to store the returned PID.
 * @param taskName  The task's name.
 *
 * @return 0 on success, -1 otherwise.
 */
int EBCL_taskDBGetTaskPID(ebcl_TaskDB_t *ctx, pid_t *pid, const char *taskName);

/**
 * Get the ebcl_TaskState_t and the PID of a task in a task database
 *
 * Will search \a ctx for an ebcl_Task_t with ebcl_Task_t::name lexicographically equal to \a taskName and write its
 * ebcl_Task_t::state to \a s and its PID to \a pid. If such a task does not exist in \a ctx, an error is returned. If
 * the task does not currently have a running process, \a pid will be -1 but the function will indicate success. The
 * function uses ebcl_TaskDB_t::lock for synchronization and is thread-safe.
 *
 * @param ctx       The ebcl_TaskDB_t context in which the task is held.
 * @param s         Pointer to store the returned ebcl_TaskState_t.
 * @param pid       Pointer to store the returned PID.
 * @param taskName  The task's name.
 *
 * @return 0 on success, -1 otherwise.
 */
int EBCL_taskDBGetTaskStateAndPID(ebcl_TaskDB_t *ctx, ebcl_TaskState_t *s, pid_t *pid, const char *taskName);

/**
 * Run ebcl_TaskDB_t::spawnFunc for each startable task in a task database.
 *
 * A task is startable if and only if it has no remaining ebcl_Task_t::deps and it has either not been started before
 * according to ebcl_Task_t::state or it should be respawned. A task should be respawned if and only if
 * ebcl_Task_t::opts contains the flag #EBCL_TASK_OPT_RESPAWN and either ebcl_Task_t::maxRetries is -1 or
 * ebcl_Task_t::failCount is less than ebcl_Task_t::maxRetries. The function uses ebcl_TaskDB_t::lock for
 * synchronization and is thread-safe.
 *
 * If ebcl_TaskDB::spawnInhibit is true, no tasks are considered startable and this function will return successfully
 * without starting anything.
 *
 * @param ctx  The TaskDB context from which tasks will be started.
 *
 * @return 0 on success, -1 otherwise
 */
int EBCL_taskDBSpawnReady(ebcl_TaskDB_t *ctx);
/**
 * Inhibit or un-inhibit spawning of processes by setting ebcl_TaskDB_t::spawnInhibit.
 *
 * The function uses ebcl_TaskDB_t::lock for synchronization and is thread-safe. It will also signal
 * ebcl_TaskDB_t::changed if ebcl_TaskDB_t::spawnInhibit was changed to false.
 *
 * @param ctx  The TaskDB context in which to set the variable.
 * @param inh  The value which ebcl_TaskDB_t:spawnInhibit shall be set to.
 *
 * @return 0 on success, -1 otherwise
 */
int EBCL_taskDBSetSpawnInhibit(ebcl_TaskDB_t *ctx, bool inh);

/**
 *  Initialize the internals of an ebcl_TaskDB_t with a specified initial size for ebcl_TaskDB_t::taskSet.
 *
 *  If the initialized TaskDB is no longer needed the internally held dynamic memory should be freed using
 *  EBCL_taskDBDestroy().
 *
 *  @param ctx          The ebcl_TaskDB_t whose internal members should be initialized.
 *  @param spawnFunc    Pointer to the task spawn function to be used by EBCL_taskDBSpawnReady()
 *  @param initialSize  Initial size of ebcl_TaskDB_t::taskSet for \a ctx.
 *
 *  @return 0 on success, -1 otherwise
 */
int EBCL_taskDBInitWithSize(ebcl_TaskDB_t *ctx, int (*spawnFunc)(ebcl_TaskDB_t *ctx, const ebcl_Task_t *),
                            size_t initialSize);
/**
 * Initialize the internals of an ebcl_TaskDB_t with the default initial size of EBCL_TASKDB_INITIAL_SIZE.
 *
 * See EBCL_taskDBInitWithSize().
 */
#define EBCL_taskDBInit(ctx, spawnFunc) EBCL_taskDBInitWithSize(ctx, spawnFunc, EBCL_TASKDB_INITIAL_SIZE);

/**
 * Free memory allocated for ebcl_TaskDB_t members by functions like EBCL_taskDBInit or EBCL_taskDBInsert().
 *
 * Afterwards \a ctx may not be used anymore until another call to EBCL_taskDBInit() or EBCL_taskDBInitWithSize().
 *
 * @param ctx The ebcl_TaskDB_t context to be destroyed.
 *
 * @return 0 on success, -1 on error
 */
int EBCL_taskDBDestroy(ebcl_TaskDB_t *ctx);

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
int EBCL_taskDBExportTaskNamesToArray(ebcl_TaskDB_t *ctx, char **tasks[], size_t *numTasks);

#endif /* __TASKDB_H__ */
