// SPDX-License-Identifier: MIT
/**
 * @file confparse.h
 * @brief Header related to the Config Parser.
 */
#ifndef __CONFPARSE_H__
#define __CONFPARSE_H__

#include <stdbool.h>
#include <sys/types.h>

#include "fseries.h"

/**  Config key for the list of task file names. **/
#define CRINIT_CONFIG_KEYSTR_TASKS "TASKS"
/**  Config key for the list of task file names. **/
#define CRINIT_CONFIG_KEYSTR_INCLUDES "INCLUDES"
/**  Config key for the option to follow symbolic links from `TASKDIR` in dynamic configurations. **/
#define CRINIT_CONFIG_KEYSTR_TASKDIR_SYMLINKS "TASKDIR_FOLLOW_SYMLINKS"
/**  Config file key for DEBUG global option. **/
#define CRINIT_CONFIG_KEYSTR_DEBUG "DEBUG"
/**  Config file key for TASKDIR global option. **/
#define CRINIT_CONFIG_KEYSTR_TASKDIR "TASKDIR"
#ifdef ENABLE_CAPABILITIES
/**  Config file key for DEFAULTCAPS global option. **/
#define CRINIT_CONFIG_KEYSTR_DEFAULTCAPS "DEFAULTCAPS"
#endif
/**  Config file key for INCLUDEDIR global option. **/
#define CRINIT_CONFIG_KEYSTR_INCLDIR "INCLUDEDIR"
/**  Config file key for SHUTDOWN_GRACE_PERIOD_US global option **/
#define CRINIT_CONFIG_KEYSTR_SHDGRACEP "SHUTDOWN_GRACE_PERIOD_US"
/**  Config file key for USE_SYSLOG global option. **/
#define CRINIT_CONFIG_KEYSTR_USE_SYSLOG "USE_SYSLOG"
/**  Config file key for USE_ELOS global option. **/
#define CRINIT_CONFIG_KEYSTR_USE_ELOS "USE_ELOS"
/**  Config file key for ELOS_SERVER global option. **/
#define CRINIT_CONFIG_KEYSTR_ELOS_SERVER "ELOS_SERVER"
/**  Config file key for ELOS_PORT global option. **/
#define CRINIT_CONFIG_KEYSTR_ELOS_PORT "ELOS_PORT"
/**  Config file key for ELOS_EVENT_POLL_INTERVAL global option. **/
#define CRINIT_CONFIG_KEYSTR_ELOS_EVENT_POLL_INTERVAL "ELOS_EVENT_POLL_INTERVAL"
/**  Config file key for LAUNCHER_CMD global option. **/
#define CRINIT_CONFIG_KEYSTR_LAUNCHER_CMD "LAUNCHER_CMD"
/**  Config file key for INCLUDE_SUFFIX global option. **/
#define CRINIT_CONFIG_KEYSTR_INCL_SUFFIX "INCLUDE_SUFFIX"
/**  Config key for the task file extension in dynamic configurations. **/
#define CRINIT_CONFIG_KEYSTR_TASK_FILE_SUFFIX "TASK_FILE_SUFFIX"

/**  Name of the option to set the public key dir from Kernel command line. **/
#define CRINIT_CONFIG_KEYSTR_SIGKEYDIR "sigkeydir"
/**  Name of the option to activate signature checking on the Kernel command line. **/
#define CRINIT_CONFIG_KEYSTR_SIGNATURES "signatures"

/**  Config key to add a command to the task. **/
#define CRINIT_CONFIG_KEYSTR_COMMAND "COMMAND"
#ifdef ENABLE_CAPABILITIES
/** Config key to clear a capability to the task. **/
#define CRINIT_CONFIG_KEYSTR_CAP_CLEAR "CAPABILITY_CLEAR"
/** Config key to add a capability to the task. **/
#define CRINIT_CONFIG_KEYSTR_CAP_SET "CAPABILITY_SET"
#endif
/**  Config key to add dependencies to the task. **/
#define CRINIT_CONFIG_KEYSTR_DEPENDS "DEPENDS"
/**  Config key to set an environment variable with. **/
#define CRINIT_CONFIG_KEYSTR_ENV_SET "ENV_SET"
/**  Config key to define an elos filter. **/
#define CRINIT_CONFIG_KEYSTR_FILTER_DEFINE "FILTER_DEFINE"
/**  Config key for file include directives. **/
#define CRINIT_CONFIG_KEYSTR_INCLUDE "INCLUDE"
/**  Config key for IO redirections. **/
#define CRINIT_CONFIG_KEYSTR_IOREDIR "IO_REDIRECT"
/**  Config key for the task name. **/
#define CRINIT_CONFIG_KEYSTR_NAME "NAME"
/**  Config key for provided features. **/
#define CRINIT_CONFIG_KEYSTR_PROVIDES "PROVIDES"
/**  Config key to set a task to be respawning. **/
#define CRINIT_CONFIG_KEYSTR_RESPAWN "RESPAWN"
/**  Config key to set how often a task is allowed to respawn on failure. **/
#define CRINIT_CONFIG_KEYSTR_RESPAWN_RETRIES "RESPAWN_RETRIES"
/**  Config key to add a stop command to the task. **/
#define CRINIT_CONFIG_KEYSTR_STOP_COMMAND "STOP_COMMAND"
/**  Config key to set a specific user to run task's commands. **/
#define CRINIT_CONFIG_KEYSTR_USER "USER"
/**  Config key to set a specific group to run task's commands. **/
#define CRINIT_CONFIG_KEYSTR_GROUP "GROUP"
#ifdef ENABLE_CGROUP
/**  Config key to set or reference a cgroup name. **/
#define CRINIT_CONFIG_CGROUP_NAME "CGROUP_NAME"
#endif

/**  Default filename extension of task files. **/
#define CRINIT_CONFIG_DEFAULT_TASK_FILE_SUFFIX ".crinit"
/**  Config key for the task include file extension in dynamic configurations. **/
#define CRINIT_CONFIG_KEYSTR_INCL_FILE_SUFFIX "INCL_FILE_SUFFIX"
/**  Default filename extension of task include files. **/
#define CRINIT_CONFIG_DEFAULT_INCL_FILE_SUFFIX ".crincl"
/**  Default value for DEBUG global option. **/
#define CRINIT_CONFIG_DEFAULT_DEBUG false
/** Default value for the `TASKDIR_FOLLOW_SYMLINKS` global option. **/
#define CRINIT_CONFIG_DEFAULT_TASKDIR_SYMLINKS true
#ifndef CRINIT_LAUNCHER_COMMAND_DEFAULT
#define CRINIT_CONFIG_DEFAULT_LAUNCHER_CMD "/usr/bin/crinit-launch"
#else
/**  Default value for LAUNCHER_CMD global option. **/
#define CRINIT_CONFIG_DEFAULT_LAUNCHER_CMD CRINIT_LAUNCHER_COMMAND_DEFAULT
#endif

#ifdef ENABLE_CAPABILITIES
/**  Default value for DEFAULTCAPS global option **/

#ifndef CRINIT_CONFIG_DEFAULT_DEFAULTCAPS
#define CRINIT_CONFIG_DEFAULT_DEFAULTCAPS ""
#endif
#endif
/**  Default value for SHUTDOWN_GRACE_PERIOD_US global option **/
#define CRINIT_CONFIG_DEFAULT_SHDGRACEP 100000uLL
/**  Default value for USE_SYSLOG global option. **/
#define CRINIT_CONFIG_DEFAULT_USE_SYSLOG false
/**  Default value for USE_ELOS global option. **/
#define CRINIT_CONFIG_DEFAULT_USE_ELOS false
/**  Default value for ELOS_SERVER global option. **/
#define CRINIT_CONFIG_DEFAULT_ELOS_SERVER "127.0.0.1"
/**  Default value for ELOS_SERVER global option. **/
#define CRINIT_CONFIG_DEFAULT_ELOS_PORT 54321
/**  Default filename extension of include files. **/
#define CRINIT_CONFIG_DEFAULT_INCL_SUFFIX ".crincl"

#define CRINIT_CONFIG_DEFAULT_SIGNATURES false

/**  What stdout is called in task configs. **/
#define CRINIT_CONFIG_STDOUT_NAME "STDOUT"
/**  What stderr is called in task configs. **/
#define CRINIT_CONFIG_STDERR_NAME "STDERR"
/**  What stdin is called in task configs. **/
#define CRINIT_CONFIG_STDIN_NAME "STDIN"

/** Enumeration of all configuration keys. Goes together with crinitTaskCfgMap and crinitSeriesCfgMap. **/
typedef enum crinitConfigs {
    CRINIT_CONFIG_COMMAND = 0,
    CRINIT_CONFIG_DEBUG,
#ifdef ENABLE_CAPABILITIES
    CRINIT_CONFIG_DEFAULTCAPS,
#endif
    CRINIT_CONFIG_DEPENDS,
    CRINIT_CONFIG_ELOS_EVENT_POLL_INTERVAL,
    CRINIT_CONFIG_ELOS_PORT,
    CRINIT_CONFIG_ELOS_SERVER,
    CRINIT_CONFIG_ENV_SET,
    CRINIT_CONFIG_FILTER_DEFINE,
    CRINIT_CONFIG_GROUP,
    CRINIT_CONFIG_INCLUDE,
    CRINIT_CONFIG_INCLUDE_SUFFIX,
    CRINIT_CONFIG_INCLUDEDIR,
    CRINIT_CONFIG_IOREDIR,
    CRINIT_CONFIG_NAME,
    CRINIT_CONFIG_PROVIDES,
    CRINIT_CONFIG_RESPAWN,
    CRINIT_CONFIG_RESPAWN_RETRIES,
    CRINIT_CONFIG_SHDGRACEP,
    CRINIT_CONFIG_SIGKEYDIR,
    CRINIT_CONFIG_SIGNATURES,
    CRINIT_CONFIG_STOP_COMMAND,
    CRINIT_CONFIG_TASK_FILE_SUFFIX,
    CRINIT_CONFIG_TASKDIR,
    CRINIT_CONFIG_TASKDIR_FOLLOW_SYMLINKS,
    CRINIT_CONFIG_TASKS,
    CRINIT_CONFIG_USE_SYSLOG,
    CRINIT_CONFIG_USE_ELOS,
    CRINIT_CONFIG_USER,
    CRINIT_CONFIG_LAUNCHER_CMD,
#ifdef ENABLE_CAPABILITIES
    CRINIT_CONFIG_CAP_CLEAR,
    CRINIT_CONFIG_CAP_SET,
#endif
    CRINIT_CONFIGS_SIZE
} crinitConfigs_t;

/** Different types of configuration sources **/
typedef enum crinitConfigType {
    CRINIT_CONFIG_TYPE_SERIES,   ///< Configurations set from the series file.
    CRINIT_CONFIG_TYPE_TASK,     ///< Configurations set from a task file.
    CRINIT_CONFIG_TYPE_KCMDLINE  ///< Configurations set from the Kernel command line.
} crinitConfigType_t;

/**
 * Linked list to hold key/value pairs read from the config file.
 */
typedef struct crinitConfKvList {
    struct crinitConfKvList *next;  ///< Pointer to next element
    char *key;                      ///< string with "KEY"
    char *val;                      ///< string with "VALUE"
} crinitConfKvList_t;

/**
 * Parse a config file into a crinitConfKvList_t.
 *
 * Parses a config file and fills \a confList. Items of \a confList are dynamically allocated (grown) and need to be
 * freed using free_confList(). The format of the config file is expected to be
 * `KEY1=VALUE1<newline>KEY2=VALUE2<newline>...` Lines beginning with `#` are considered comments.
 *
 * If the Kernel command line option `crinit.signatures` is set to `yes`, this function will also check the
 * configuration file's signature. A non-matching signature is handled as a parser error.
 *
 * @param confList  will return a pointer to dynamically allocated memory of a ConfKvList filled with the
 *                  key/value-pairs from the config file.
 * @param filename  Path to the configuration file.
 *
 * @return 0 on success, -1 on error
 *
 */
int crinitParseConf(crinitConfKvList_t **confList, const char *filename);

/**
 * Frees memory allocated for an crinitConfKvList_t by crinitParseConf().
 *
 * @param confList  Pointer to crinitConfKvList_t allocated by crinitParseConf() and not freed before. If confList is
 *                  NULL, crinitFreeConfList() will return without freeing any memory.
 */
void crinitFreeConfList(crinitConfKvList_t *confList);

/**
 * Frees a string array with a backing string.
 *
 * @param inArgv   The string array to free, must be a double pointer with 2 allocations, one array of pointers and a
 *                 single inner array of char.
 */
void crinitFreeArgvArray(char **inArgv);

/**
 * Parse a series file.
 *
 * Will return the task config and include files to be loaded in \a series. Will also set any global options specified
 * in the series file.
 *
 * @param filename    The path to the series file to load.
 *
 * @return 0 on success, -1 on failure
 */
int crinitLoadSeriesConf(const char *filename);

/**
 * Load all tasks related to a series file.
 *
 * @param series      Returns the paths to the task configs specified in the series file (or scanned from TASKDIR, if
 *                    configured).
 *
 * @return 0 on success, -1 on failure
 */
int crinitLoadTasks(crinitFileSeries_t *series);

#endif /* __CONFPARSE_H__ */
