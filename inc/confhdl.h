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
 * Each configuration directive has its own handler function implementation, mapped by EBCL_cfgMap. It will use the
 * appropriate parsing/conversion functions (see confconv.h) to extract desired settings and then write them to the
 * \a tgt crinitTask_t.
 *
 * @param tgt  The task to write the new settings to.
 * @param val  The new setting to parse.
 *
 * @return  0 on success, -1 on error
 */
typedef int (*ebcl_ConfigHandler_t)(crinitTask_t *tgt, const char *val);

/** Handler for `COMMAND` config directives. See ebcl_ConfigHandler_t. **/
int EBCL_taskCfgCmdHandler(crinitTask_t *tgt, const char *val);
/** Handler for `DEPENDS` config directives. See ebcl_ConfigHandler_t. **/
int EBCL_taskCfgDepHandler(crinitTask_t *tgt, const char *val);
/** Handler for `ENV_SET` config directives. See ebcl_ConfigHandler_t. **/
int EBCL_taskCfgEnvHandler(crinitTask_t *tgt, const char *val);
/** Handler for `IO_REDIRECT` config directives. See ebcl_ConfigHandler_t. **/
int EBCL_taskCfgIoRedirHandler(crinitTask_t *tgt, const char *val);
/** Handler for `NAME` config directives. See ebcl_ConfigHandler_t. **/
int EBCL_taskCfgNameHandler(crinitTask_t *tgt, const char *val);
/** Handler for `PROVIDES` config directives. See ebcl_ConfigHandler_t. **/
int EBCL_taskCfgPrvHandler(crinitTask_t *tgt, const char *val);
/** Handler for `RESPAWN` config directives. See ebcl_ConfigHandler_t. **/
int EBCL_taskCfgRespHandler(crinitTask_t *tgt, const char *val);
/** Handler for `RESPAWN_RETRIES` config directives. See ebcl_ConfigHandler_t. **/
int EBCL_taskCfgRespRetHandler(crinitTask_t *tgt, const char *val);
/** Handler for `INCLUDE` config directives. See ebcl_ConfigHandler_t. **/
int EBCL_taskIncludeHandler(crinitTask_t *tgt, const char *val);

#endif /* __CONFHDL_H__ */
