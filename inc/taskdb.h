/**
 * @file taskdb.h
 * @brief Header defining the central Task Data Structure and related types/functions.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __TASKDB_H__
#define __TASKDB_H__

#include <pthread.h>
#include <stddef.h>

#include "confparse.h"
#include "crinit-sdefs.h"

#define EBCL_MONITOR_DEP_NAME "@ebclmon"  ///< Special dependency name to depend on monitor events (not yet impl.).
#define EBCL_TASKDB_INITIAL_SIZE 256      ///< Default initial size of the taskSet within an ebcl_TaskDB_t.

typedef unsigned long ebcl_TaskOpts_t;  ///< Type to store Task option bitmask.

#define EBCL_TASK_OPT_EXEC (1 << 0)     ///< EXEC task option bitmask (not yet implemnted).
#define EBCL_TASK_OPT_QM_JAIL (1 << 1)  ///< QM_JAIL task option bitmask (not yet implemented).
#define EBCL_TASK_OPT_RESPAWN (1 << 2)  ///< RESPAWN task option bitmask.

#define EBCL_TASK_EVENT_RUNNING "spawn"  ///< Dependency event that fires when a task reaches the RUNNING state.
#define EBCL_TASK_EVENT_DONE "wait"      ///< Dependency event that fires when a task reaches the DONE state.
#define EBCL_TASK_EVENT_FAILED "fail"    ///< Dependency event that fires when a task reaches the FAILED state.

/**
 * Type to store a single command within a task.
 */
typedef struct ebcl_TaskCmd_t {
    int argc;     ///< Number of arguments within argv.
    char **argv;  ///< String array containing the program arguments, argv[0] contains absolute path to executable.
} ebcl_TaskCmd_t;

/**
 * Type to store a single dependency within a task.
 */
typedef struct ebcl_TaskDep_t {
    char *name;   ///< Dependency name.
    char *event;  ///< Dependency event.
} ebcl_TaskDep_t;

/**
 * Type to store a single task.
 */
typedef struct ebcl_Task_t {
    char *name;              ///< Name of the task, corresponds to NAME in the config file.
    ebcl_TaskCmd_t *cmds;    ///< Dynamic array of commands, corresponds to COMMAND[N] in the config file.
    size_t cmdsSize;         ///< Number of commands in cmds array.
    ebcl_TaskDep_t *deps;    ///< Dynamic array of dependencies, corresponds to DEPENDS in the config file.
    size_t depsSize;         ///< Number of dependencies in deps array.
    ebcl_TaskOpts_t opts;    ///< Task options.
    ebcl_TaskState_t state;  ///< Task state.
    pid_t pid;               ///< PID of currently running process subordinate to the task, if any.
} ebcl_Task_t;

/**
 * Type to store a task database.
 */
typedef struct ebcl_TaskDB_t {
    ebcl_Task_t *taskSet;  ///< Dynamic array of tasks, corresponds to task configs specified in the series config.
    size_t taskSetSize;    ///< Current maximum size of the task array.
    size_t taskSetItems;   ///< Number of elements in the task array.

    /** Pointer specifying a function for spawning ready tasks, used by EBCL_taskDBSpawnReady() **/
    int (*spawnFunc)(struct ebcl_TaskDB_t *ctx, const ebcl_Task_t *);

    pthread_mutex_t lock;    ///< Mutex to lock the TaskDB, shall be used for any operations on the taskSet.
    pthread_cond_t changed;  ///< Condition variable to be signalled if taskSet is changed.
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
 * according to strcmp()) and, if found, remove the dependency from ebcl_TaskDB_t::deps. Will signal
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
 * Add a dependency to a specific task inside a task database.
 *
 * Will search \a ctx for a task with name \a taskName and add \a dep to its ebcl_TaskDB_t::deps and adjust
 * ebcl_TaskDB_t::depsSize.
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
 * ebcl_TaskDB_t::deps and adjust ebcl_TaskDB_t::depsSize, if such a dependency is present. The equality condition
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
 * Will search \a ctx for an ebcl_Task_t with ebcl_TaskDB_t::name lexicographically equal to \a taskName and set its
 * ebcl_TaskDB_t::state to \a s. If such a task does not exit in \a ctx, an error is returned. The function uses
 * ebcl_TaskDB_t::lock for synchronization and is thread-safe.
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
 * Will search \a ctx for an ebcl_Task_t with ebcl_TaskDB_t::name lexicographically equal to \a taskName and write its
 * ebcl_TaskDB_t::state to \a s. If such a task does not exit in \a ctx, an error is returned. The function uses
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
 * Will search \a ctx for an ebcl_Task_t with ebcl_TaskDB_t::name lexicographically equal to \a taskName and set its
 * ebcl_TaskDB_t::pid to \a pid. If such a task does not exit in \a ctx, an error is returned. The function uses
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
 * Will search \a ctx for an ebcl_Task_t with ebcl_TaskDB_t::name lexicographically equal to \a taskName and write its
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
 * Set the process spawning function of the TaskDB.
 *
 * Setting \a spawnFunc to NULL will inhibit spawning of new processes.
 *
 * @param ctx        The ebcl_TaskDB context for which to set the spawning function.
 * @param spawnFunc  The new spawning function to set, may be NULL.
 *
 * @return 0 on success, -1 on error
 */
int EBCL_taskDBSetSpawnFunc(ebcl_TaskDB_t *ctx, int (*spawnFunc)(ebcl_TaskDB_t *ctx, const ebcl_Task_t *));

/**
 * Run ebcl_TaskDB_t::spawnFunc for each startable task in a task database.
 *
 * A task is startable if and only if it has no remaining ebcl_TaskDB_t::deps and it has not been started before
 * according to ebcl_TaskDB_t::state. The function uses ebcl_TaskDB_t::lock for synchronization and is thread-safe.
 *
 * @param ctx  The TaskDB context from which tasks will be started.
 *
 * @return 0 on success, -1 otherwise
 */
int EBCL_taskDBSpawnReady(ebcl_TaskDB_t *ctx);

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
 * Given an ebcl_ConfKvList_t created from a task config, build an equivalent ebcl_Task.
 *
 * The ebcl_Task returned via \a out is dynamically allocated and should be freed using EBCL_freeTask if no longer
 * needed.
 *
 * @param out  The return pointer for the newly created task.
 * @param in   The ebcl_ConfKvList_t from which to build the task.
 *
 * @return 0 on success, -1 on error
 */
int EBCL_taskCreateFromConfKvList(ebcl_Task_t **out, const ebcl_ConfKvList_t *in);

/**
 * Frees memory associated with an ebcl_Task created by EBCL_taskCreateFromConfKvList() or EBCL_taskDup().
 *
 * @param t  Pointer to the ebcl_Task to free.
 */
void EBCL_freeTask(ebcl_Task_t *t);

/**
 *  Duplicates an ebcl_Task.
 *
 *  The copy returned via \a out is dynamically allocated and should be freed using EBCL_freeTask() if no longer needed.
 *
 *  @param out   Double pointer to return a dynamically allocated copy of \a orig.
 *  @param orig  The original task to copy.
 *
 *  @return 0 on success, -1 on error
 */
int EBCL_taskDup(ebcl_Task_t **out, const ebcl_Task_t *orig);

#endif /* __TASKDB_H__ */
