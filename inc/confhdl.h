/**
 * @file confhdl.h
 * @brief Header defining type- and target-specific handler functions for configuration parsing.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __CONFHDL_H__
#define __CONFHDL_H__

#include "task.h"

/**
 * Function pointer type definition for configuration directive handlers.
 *
 * Each configuration directive has its own handler function implementation, mapped by crinitCfgMap. It will use the
 * appropriate parsing/conversion functions (see confconv.h) to extract desired settings and then write them to the
 * \a tgt crinitTask_t.
 *
 * @param tgt  The task to write the new settings to.
 * @param val  The new setting to parse.
 *
 * @return  0 on success, -1 on error
 */
typedef int (*crinitConfigHandler_t)(void *tgt, const char *val, crinitConfigType_t type);

/** Handler for `COMMAND` config directives. See crinitConfigHandler_t. **/
int crinitCfgCmdHandler(void *tgt, const char *val, crinitConfigType_t type);
/** Handler for `DEPENDS` config directives. See crinitConfigHandler_t. **/
int crinitCfgDepHandler(void *tgt, const char *val, crinitConfigType_t type);
/** Handler for `ENV_SET` config directives. See crinitConfigHandler_t. **/
int crinitCfgEnvHandler(void *tgt, const char *val, crinitConfigType_t type);
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

int crinitCfgDebugHandler(void *tgt, const char *val, crinitConfigType_t type);
int crinitCfgInclSuffixHandler(void *tgt, const char *val, crinitConfigType_t type);
int crinitCfgInclDirHandler(void *tgt, const char *val, crinitConfigType_t type);
int crinitCfgShdGpHandler(void *tgt, const char *val, crinitConfigType_t type);
int crinitCfgTaskSuffixHandler(void *tgt, const char *val, crinitConfigType_t type);
int crinitCfgTaskDirHandler(void *tgt, const char *val, crinitConfigType_t type);
int crinitCfgTaskDirSlHandler(void *tgt, const char *val, crinitConfigType_t type);
int crinitCfgTasksHandler(void *tgt, const char *val, crinitConfigType_t type);
int crinitCfgSyslogHandler(void *tgt, const char *val, crinitConfigType_t type);

#endif /* __CONFHDL_H__ */
