// SPDX-License-Identifier: MIT
/**
 * @file crinit-client.h
 * @brief Public definitions/declarations for using the crinit-client shared library.
 */
#ifndef __CRINIT_CLIENT_H__
#define __CRINIT_CLIENT_H__

#include <bits/types/FILE.h>
#include <sched.h>
#include <stdarg.h>
#include <stdbool.h>

#include "crinit-sdefs.h"
#include "version.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Selects the stream on which to output error messages.
 *
 * By default, the crinit-client library will output error messages to stderr specifying where a failure occurred. This
 * function can be used to pipe them to a logfile instead or to completely suppress them by piping to /dev/null. The
 * stream can be the same as the one used for crinitClientSetInfoStream().
 *
 * @param errStream  The stream to use.
 */
void crinitClientSetErrStream(FILE *errStream);
/**
 * Selects the stream on which to output (debug) information messages.
 *
 * By default, the crinit-client library will output debug information messages to stdout if activated by
 * crinitClientSetVerbose(). This function can be used to pipe them to a logfile instead. The stream can be the same as
 * the one used for crinitClientSetInfoStream().
 *
 * @param infoStream  The stream to use.
 */
void crinitClientSetInfoStream(FILE *infoStream);

/**
 * Queries the version of the crinit daemon.
 *
 * See also crinitVersion_t. Depending on the build environment, crinitVersion_t::git may be an empty string.
 *
 * @param v  Return pointer to an crinitVersion_t in which the crinit daemon's version will be written if the query is
 * successful.
 *
 * @return 0 on success, -1 otherwise
 */
int crinitClientGetVersion(crinitVersion_t *v);
/**
 * Returns version information for the crinit-client shared library.
 *
 * See also crinitVersion_t. Depending on the build environment, crinitVersion_t::git may be an empty string.
 *
 * @return  A pointer to an crinitVersion_t constant containing this library's version info.
 */
const crinitVersion_t *crinitClientLibGetVersion(void);

/**
 * Sets the task name reported to Crinit by sd_notify().
 *
 * The default is set via the environment variable specified by #CRINIT_ENV_NOTIFY_NAME if it is present.
 *
 * @param taskName  The task name to use.
 */
void crinitClientSetNotifyTaskName(const char *taskName);
/**
 * Sets the path to Crinit's AF_UNIX communication socket.
 *
 * The default is set by #CRINIT_SOCKFILE.
 *
 * @param sockFile  Path to the socket file.
 */
void crinitClientSetSocketPath(const char *sockFile);
/**
 * Turns debug output on or off.
 *
 * The default is no debug output.
 *
 * @param v  If there should be debug output (YES==true).
 */
int crinitClientSetVerbose(bool v);

/**
 * Notifies Crinit of task state changes.
 *
 * Partially implements the SD_NOTIFY interface of systemd. Specifically, the commands READY and MAINPID are supported.
 * Others are currently unimplemented and will be ignored. The \a unset_environment argument is also unimplemented, i.e.
 * the environment will not be unset. If \a unset_environment is not 0, a warning will be printed.
 *
 * READY=1 lets Crinit know the task is currently running. MAINPID=[pid] tells Crinit its PID. Delimiting character is a
 * newline.
 *
 * Example: `"READY=1\nMAINPID=42"` will update the task's state to #CRINIT_TASK_STATE_RUNNING and its PID to 42.
 *
 * For more information regarding the use cases of this interface, refer to the official SD_NOTIFY documentation.
 *
 * @param unset_environment  Unimplemented, should be set to 0.
 * @param state              SD_NOTIFY string. A newline-separated list of commands, as in the above example.
 *
 * @return 0 on success, -1 otherwise
 */
int sd_notify(int unset_environment, const char *state);  // NOLINT(readability-identifier-naming)
                                                          // Rationale: Naming determined by external API.
/**
 * Notifies Crinit of task state changes.
 *
 * Like sd_notify() but uses printf-style formatting for the input string.
 */
int sd_notifyf(int unset_environment, const char *format, ...);  // NOLINT(readability-identifier-naming)
                                                                 // Rationale: Naming determined by external API.

/**
 * Requests Crinit to add a task from a given task config.
 *
 * If successful, the given task configuration will be parsed and added to the TaskDB. Task execution takes place once
 * all dependencies have been fulfilled, as with pre-loaded tasks.
 *
 * Using \a forceDeps, it is possible to specify a DEPENDS value which overrides the one in the configuration file. An
 * empty string will cause the task to be started immediately. Specifying `@ctl:enable` will let the task wait for
 * crinitClientTaskEnable(). If the dependencies from the task config file should be used, \a forceDeps must be `NULL`
 * or `"@unchanged"`.
 *
 * Note, that Crinit does not keep track of already fulfilled dependencies, i.e. in order to not be blocked forever a
 * task configuration must only contain dependencies which will be fulfilled **in the future**.
 *
 * @param configFilePath   **Absolute** path to the task configuration file.
 * @param overwrite        If an existing task with the same name should be overwritten (==true). Otherwise Crinit would
 *                         return an error in this case.
 * @param forceDeps        String to override the value of DEPENDS in the config file, can be "" for no dependencies
 *                         (immediate start), an arbitrary list of dependencies, or `NULL`/`"@unchanged"` to keep the
 *                         dependencies in the config.
 *
 * @return 0 on success, -1 otherwise
 */
int crinitClientTaskAdd(const char *configFilePath, bool overwrite, const char *forceDeps);
/**
 * Requests Crinit to load tasks and options from a series file.
 *
 * If successful, the task configurations and options in the series file will be loaded same as if the series file was
 * specified on startup. Already loaded tasks from a prior series file or loaded via crinit-ctl with the same names will
 * be overwritten if and only if \a overwriteTasks is set to `true`. Otherwise, task name collisions are an error.
 *
 * Config options from the newly loaded file generally take precedence over the existing values.
 *
 * Crinit will spawn no new processes during loading of a series file in order to preserve ordering through
 * dependencies.
 *
 * @param seriesFilePath  **Absolute** path to the series file.
 * @param overwriteTasks  If true, tasks with the same name already in the TaskDB will be overwritten.
 *
 * @return  0 on success, -1 otherwise
 */
int crinitClientSeriesAdd(const char *seriesFilePath, bool overwriteTasks);

/**
 * Requests Crinit to remove the dependency `"@ctl:enable"` from a specific task.
 *
 * This can be used in tandem with crinitClientTaskDisable() or config files which already contain the above dependency
 * to implement starting on command.
 *
 * @param taskName  The name of the task.
 *
 * @return 0 on success, -1 on error
 */
int crinitClientTaskEnable(const char *taskName);
/**
 * Requests Crinit to add the dependency `"@ctl:enable"` to a specific task.
 *
 * This can be used in tandem with crinitClientTaskEnable() to implement starting on command.
 *
 * Note, that this function can also be used to prevent tasks with the `RESPAWN` option set to `YES` from respawning,
 * temporarily or permanently.
 *
 * @param taskName  The name of the task.
 *
 * @return 0 on success, -1 on error
 */
int crinitClientTaskDisable(const char *taskName);
/**
 * Request Crinit to send SIGTERM to a task process.
 *
 * Will only send a signal if a corresponding PID has been saved in the TaskDB, either through sd_notify() or the
 * Process Dispatcher during process start.
 *
 * @param taskName  The name of the task.
 *
 * @return 0 on success, -1 on error
 */
int crinitClientTaskStop(const char *taskName);
/**
 * Request Crinit to send SIGKILL to a task process.
 *
 * Will only send a signal if a corresponding PID has been saved in the TaskDB, either through sd_notify() or the
 * Process Dispatcher during process start.
 *
 * @param taskName  The name of the task.
 *
 * @return 0 on success, -1 on error
 */
int crinitClientTaskKill(const char *taskName);
/**
 * Request Crinit to reset the state of a task within the TaskDB.
 *
 * State will be reset to 0 it is currently #CRINIT_TASK_STATE_DONE or #CRINIT_TASK_STATE_FAILED. If the task has no
 * unfulfilled dependencies, this will cause an immediate restart.
 *
 * Note, that the semantics are different than with e.g. `systemctl restart`. To get equivalent behaviour, i.e. restart
 * a task that is currently running, a possibility is a call to crinitClientTaskStop() followed by a call to this
 * function.
 *
 * @param taskName  The name of the task.
 *
 * @return 0 on success, -1 on error
 */
int crinitClientTaskRestart(const char *taskName);
/**
 * Request Crinit to report the current state, PID, and timestamps of a task from its TaskDB.
 *
 * Timestamps of events that have not yet happened are always `{0,0}`.
 *
 * @param s         Return pointer for the task's state.
 * @param pid       Return pointer for the task's PID.
 * @param ct        The time when the task was created (loaded/parsed).
 * @param st        The time when the task was last started (i.e. became running).
 * @param et        The time when the task last ended (i.e. failed or was done).
 * @param gid       Return pointer for the task's GID.
 * @param uid       Return pointer for the task's UID.
 * @param groupname Return pointer for the task's groupname.
 * @param username  Return pointer for the task's username.
 * @param taskName  The name of the task.
 *
 * @return 0 on success, -1 on error
 */
int crinitClientTaskGetStatus(crinitTaskState_t *s, pid_t *pid, struct timespec *ct, struct timespec *st,
                              struct timespec *et, gid_t *gid, uid_t *uid, char **username, char **groupname,
                              const char *taskName);
/**
 * Request Crinit to report the list of task names from its TaskDB.
 *
 * The returned object should be freed with crinitClientFreeTaskList().
 *
 * @param tl    Return pointer for the list of tasks.
 *
 * @return 0 on success, -1 on error
 */
int crinitClientGetTaskList(crinitTaskList_t **tl);
/**
 * Free the list of tasks obtained from crinitClientGetTaskList().
 *
 * @param tl    The list of tasks.
 */
void crinitClientFreeTaskList(crinitTaskList_t *tl);
/**
 * Request Crinit to initiate an immediate shutdown or reboot.
 *
 * Calling process must have the CAP_SYS_BOOT capability or Crinit will refuse with "Permission denied."
 * Before issuing the shutdown or reboot syscall, Crinit will send SIGCONT+SIGTERM to all processes, wait a grace period
 * (which can be set in the main series config file as `SHUTDOWN_GRACE_PERIOD_US = <microseconds>`), send SIGKILL to
 * remaining processes, detach or read-only-remount all filesystems, and finally call sync().
 *
 * @param sCmd  The shutdown command to be performed, either CRINIT_SHD_POWEROFF (for poweroff) or CRINIT_SHD_POWEROFF
 *              (for reboot).
 *
 * @return 0 on success, -1 on error
 */
int crinitClientShutdown(crinitShutdownCmd_t sCmd);

#ifdef __cplusplus
}
#endif

#endif /* __CRINIT_CLIENT_H__  */
