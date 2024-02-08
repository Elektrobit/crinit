// SPDX-License-Identifier: MIT
/**
 * @file confhdl.h
 * @brief Header defining type- and target-specific handler functions for configuration parsing.
 */
#ifndef __CONFHDL_H__
#define __CONFHDL_H__

#include "task.h"

/**
 * Function pointer type definition for configuration directive handlers.
 *
 * Each configuration directive has its own handler function implementation, mapped by crinit{Task,Series}CfgMap. It
 * will use the appropriate parsing/conversion functions (see confconv.h) to extract desired settings and then write
 * them to the target or the global option storage.
 *
 * @param tgt   The target to write to. In case of a task-specific option, this is a pointer to the relevant
 *              crinitTask_t. For the `TASKS` global option it is a pointer to an array of strings. For `TASKDIR` and
 *              `TASK_SUFFIX`, it is a pointer to a string. For `TASKDIR_FOLLOW_SYMLINKS`, it is a pointer to a bool.
 *              For all other series file option, it should be NULL.
 * @param val   The new setting to parse.
 * @param type  crinitConfigType_t indicating if we are parsing a task or a series file.
 *
 * @return  0 on success, -1 on error
 */
typedef int (*crinitConfigHandler_t)(void *tgt, const char *val, crinitConfigType_t type);

/* Handlers for task-specific options */

/** Handler for `COMMAND` config directives. See crinitConfigHandler_t. **/
int crinitCfgCmdHandler(void *tgt, const char *val, crinitConfigType_t type);
/** Handler for `DEPENDS` config directives. See crinitConfigHandler_t. **/
int crinitCfgDepHandler(void *tgt, const char *val, crinitConfigType_t type);
/** Handler for `IO_REDIRECT` config directives. See crinitConfigHandler_t. **/
int crinitCfgIoRedirHandler(void *tgt, const char *val, crinitConfigType_t type);
/** Handler for `NAME` config directives. See crinitConfigHandler_t. **/
int crinitCfgNameHandler(void *tgt, const char *val, crinitConfigType_t type);
/** Handler for `PROVIDES` config directives. See crinitConfigHandler_t. **/
int crinitCfgPrvHandler(void *tgt, const char *val, crinitConfigType_t type);
/** Handler for `RESPAWN` config directives. See crinitConfigHandler_t. **/
int crinitCfgRespHandler(void *tgt, const char *val, crinitConfigType_t type);
/** Handler for `RESPAWN_RETRIES` config directives. See crinitConfigHandler_t. **/
int crinitCfgRespRetHandler(void *tgt, const char *val, crinitConfigType_t type);
/** Handler for `INCLUDE` config directives. See crinitConfigHandler_t. **/
int crinitTaskIncludeHandler(void *tgt, const char *val, crinitConfigType_t type);

/* Handlers for global options */

/** Handler for `DEBUG` config directives. See crinitConfigHandler_t. **/
int crinitCfgDebugHandler(void *tgt, const char *val, crinitConfigType_t type);
/** Handler for `INCLUDE_SUFFIX` config directives. See crinitConfigHandler_t. **/
int crinitCfgInclSuffixHandler(void *tgt, const char *val, crinitConfigType_t type);
/** Handler for `INCLUDEDIR` config directives. See crinitConfigHandler_t. **/
int crinitCfgInclDirHandler(void *tgt, const char *val, crinitConfigType_t type);
/** Handler for `SHUTDOWN_GRACE_PERIOD_US` config directives. See crinitConfigHandler_t. **/
int crinitCfgShdGpHandler(void *tgt, const char *val, crinitConfigType_t type);
/** Handler for `TASK_SUFFIX` config directives. See crinitConfigHandler_t. **/
int crinitCfgTaskSuffixHandler(void *tgt, const char *val, crinitConfigType_t type);
/** Handler for `TASKDIR` config directives. See crinitConfigHandler_t. **/
int crinitCfgTaskDirHandler(void *tgt, const char *val, crinitConfigType_t type);
/** Handler for `TASKDIR_FOLLOW_SYMLINKS` config directives. See crinitConfigHandler_t. **/
int crinitCfgTaskDirSlHandler(void *tgt, const char *val, crinitConfigType_t type);
/** Handler for `TASKS` config directives. See crinitConfigHandler_t. **/
int crinitCfgTasksHandler(void *tgt, const char *val, crinitConfigType_t type);
/** Handler for `USE_SYSLOG` config directives. See crinitConfigHandler_t. **/
int crinitCfgSyslogHandler(void *tgt, const char *val, crinitConfigType_t type);
/** Handler for `USE_ELOS` config directives. See crinitConfigHandler_t. **/
int crinitCfgElosHandler(void *tgt, const char *val, crinitConfigType_t type);
/** Handler for `ELOS_SERVER` config directives. See crinitConfigHandler_t. **/
int crinitCfgElosServerHandler(void *tgt, const char *val, crinitConfigType_t type);
/** Handler for `ELOS_PORT` config directives. See crinitConfigHandler_t. **/
int crinitCfgElosPortHandler(void *tgt, const char *val, crinitConfigType_t type);

/* Handlers working in both cases */

/** Handler for `ENV_SET` config directives. See crinitConfigHandler_t. **/
int crinitCfgEnvHandler(void *tgt, const char *val, crinitConfigType_t type);
/** Handler for `FILTER_DEFINE` config directives. See crinitConfigHandler_t. **/
int crinitCfgFilterHandler(void *tgt, const char *val, crinitConfigType_t type);

/* Handlers for parsing the Kernel command line */

/** Handler for `crinit.sigkeydir` Kernel command line setting. See crinitConfigHandler_t. **/
int crinitCfgSigKeyDirHandler(void *tgt, const char *val, crinitConfigType_t type);
/** Handler for `crinit.signatures` Kernel command line setting. See crinitConfigHandler_t. **/
int crinitCfgSignaturesHandler(void *tgt, const char *val, crinitConfigType_t type);

#endif /* __CONFHDL_H__ */
