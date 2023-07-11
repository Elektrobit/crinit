/**
 * @file confparse.h
 * @brief Header related to the Config Parser.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __CONFPARSE_H__
#define __CONFPARSE_H__

#include <stdbool.h>
#include <sys/types.h>

#include "fseries.h"

#define CRINIT_CONFIG_KEYSTR_TASKS "TASKS"        ///< Config key for the list of task file names.
#define CRINIT_CONFIG_KEYSTR_INCLUDES "INCLUDES"  ///< Config key for the list of task file names.
#define CRINIT_CONFIG_KEYSTR_TASKDIR_SYMLINKS \
    "TASKDIR_FOLLOW_SYMLINKS"  ///< Config key for the option to follow symbolic links from `TASKDIR` in dynamic
                               ///< configurations.
#define CRINIT_CONFIG_KEYSTR_DEBUG "DEBUG"         ///< Config file key for DEBUG global option.
#define CRINIT_CONFIG_KEYSTR_TASKDIR "TASKDIR"     ///< Config file key for TASKDIR global option.
#define CRINIT_CONFIG_KEYSTR_INCLDIR "INCLUDEDIR"  ///< Config file key for INCLUDEDIR global option.
#define CRINIT_CONFIG_KEYSTR_SHDGRACEP \
    "SHUTDOWN_GRACE_PERIOD_US"                        ///< Config file key for SHUTDOWN_GRACE_PERIOD_US global option
#define CRINIT_CONFIG_KEYSTR_USE_SYSLOG "USE_SYSLOG"  ///< Config file key for USE_SYSLOG global option.
#define CRINIT_CONFIG_KEYSTR_INCL_SUFFIX "INCLUDE_SUFFIX"  ///< Config file key for INCLUDE_SUFFIX global option.
#define CRINIT_CONFIG_KEYSTR_TASK_FILE_SUFFIX \
    "TASK_FILE_SUFFIX"  ///< Config key for the task file extension in dynamic configurations.

#define CRINIT_CONFIG_KEYSTR_COMMAND "COMMAND"      ///< Config key to add a command to the task.
#define CRINIT_CONFIG_KEYSTR_DEPENDS "DEPENDS"      ///< Config key to add dependencies to the task.
#define CRINIT_CONFIG_KEYSTR_ENV_SET "ENV_SET"      ///< Config key to set an environment variable with.
#define CRINIT_CONFIG_KEYSTR_INCLUDE "INCLUDE"      ///< Config key for file include directives.
#define CRINIT_CONFIG_KEYSTR_IOREDIR "IO_REDIRECT"  ///< Config key for IO redirections.
#define CRINIT_CONFIG_KEYSTR_NAME "NAME"            ///< Config key for the task name.
#define CRINIT_CONFIG_KEYSTR_PROVIDES "PROVIDES"    ///< Config key for provided features.
#define CRINIT_CONFIG_KEYSTR_RESPAWN "RESPAWN"      ///< Config key to set a task to be respawning.
#define CRINIT_CONFIG_KEYSTR_RESPAWN_RETRIES \
    "RESPAWN_RETRIES"  ///< Config key to set how often a task is allowed to respawn on failure.

#define CRINIT_CONFIG_DEFAULT_TASK_FILE_SUFFIX ".crinit"  ///< Default filename extension of task files.
#define CRINIT_CONFIG_KEYSTR_INCL_FILE_SUFFIX \
    "INCL_FILE_SUFFIX"  ///< Config key for the task include file extension in dynamic configurations.
#define CRINIT_CONFIG_DEFAULT_INCL_FILE_SUFFIX ".crincl"  ///< Default filename extension of task include files.
#define CRINIT_CONFIG_DEFAULT_DEBUG false                 ///< Default value for DEBUG global option.
#define CRINIT_CONFIG_DEFAULT_TASKDIR "/etc/crinit"       ///< Default value for TASKDIR global option.
#define CRINIT_CONFIG_DEFAULT_TASKDIR_SYMLINKS true

#define CRINIT_CONFIG_DEFAULT_INCLDIR "/etc/crinit"  ///< Default value for INCLUDEDIR global option.
#define CRINIT_CONFIG_DEFAULT_SHDGRACEP 100000uLL    ///< Default value for SHUTDOWN_GRACE_PERIOD_US global option
#define CRINIT_CONFIG_DEFAULT_USE_SYSLOG false       ///< Default value for USE_SYSLOG global option.
#define CRINIT_CONFIG_DEFAULT_INCL_SUFFIX ".crincl"  ///< Default filename extension of include files.

#define CRINIT_CONFIG_STDOUT_NAME "STDOUT"  ///< What stdout is called in task configs.
#define CRINIT_CONFIG_STDERR_NAME "STDERR"  ///< What stderr is called in task configs.
#define CRINIT_CONFIG_STDIN_NAME "STDIN"    ///< What stdin is called in task configs.

/** Enumeration of all configuration keys. Goes together with crinitTaskCfgMap and crinitSeriesCfgMap. **/
typedef enum crinitConfigs_t {
    CRINIT_CONFIG_COMMAND = 0,
    CRINIT_CONFIG_DEBUG,
    CRINIT_CONFIG_DEPENDS,
    CRINIT_CONFIG_ENV_SET,
    CRINIT_CONFIG_INCLUDE,
    CRINIT_CONFIG_INCLUDE_SUFFIX,
    CRINIT_CONFIG_INCLUDEDIR,
    CRINIT_CONFIG_IOREDIR,
    CRINIT_CONFIG_NAME,
    CRINIT_CONFIG_PROVIDES,
    CRINIT_CONFIG_RESPAWN,
    CRINIT_CONFIG_RESPAWN_RETRIES,
    CRINIT_CONFIG_SHDGRACEP,
    CRINIT_CONFIG_TASK_FILE_SUFFIX,
    CRINIT_CONFIG_TASKDIR,
    CRINIT_CONFIG_TASKDIR_FOLLOW_SYMLINKS,
    CRINIT_CONFIG_TASKS,
    CRINIT_CONFIG_USE_SYSLOG,
    CRINIT_CONFIGS_SIZE
} crinitConfigs_t;

typedef enum crinitConfigType_t { CRINIT_CONFIG_TYPE_SERIES, CRINIT_CONFIG_TYPE_TASK } crinitConfigType_t;

/**
 * Linked list to hold key/value pairs read from the config file.
 */
typedef struct crinitConfKvList_t {
    struct crinitConfKvList_t *next;  ///< Pointer to next element
    char *key;                        ///< string with "KEY"
    char *val;                        ///< string with "VALUE"
} crinitConfKvList_t;

/**
 * Parse a config file into a crinitConfKvList_t.
 *
 * Parses a config file and fills \a confList. Items of \a confList are dynamically allocated
 * (grown) and need to be freed using free_confList(). The format of the config file is expected
 * to be KEY1=VALUE1<newline>KEY2=VALUE2<newline>... Lines beginning with '#' are considered
 * comments.
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
 * @param confList Pointer to crinitConfKvList_t allocated by crinitParseConf() and not freed before. If
 *                 confList is NULL, crinitFreeConfList() will return without freeing any memory.
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
 * @param series      Returns the paths to the task configs specified in the series file (or scanned from TASKDIR, if
 *                    configured).
 * @param filename    The path to the series file to load.
 *
 * @return 0 on success, -1 on failure
 */
int crinitLoadSeriesConf(crinitFileSeries_t *series, const char *filename);

#endif /* __CONFPARSE_H__ */
