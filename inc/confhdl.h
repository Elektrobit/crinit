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

typedef int (*ebcl_ConfigHandler_t)(ebcl_Task_t *tgt, const char *val);

int EBCL_taskCfgCmdHandler(ebcl_Task_t *tgt, const char *val);
int EBCL_taskCfgDepHandler(ebcl_Task_t *tgt, const char *val);
int EBCL_taskCfgEnvHandler(ebcl_Task_t *tgt, const char *val);
int EBCL_taskCfgIoRedirHandler(ebcl_Task_t *tgt, const char *val);
int EBCL_taskCfgNameHandler(ebcl_Task_t *tgt, const char *val);
int EBCL_taskCfgPrvHandler(ebcl_Task_t *tgt, const char *val);
int EBCL_taskCfgRespHandler(ebcl_Task_t *tgt, const char *val);
int EBCL_taskCfgRespRetHandler(ebcl_Task_t *tgt, const char *val);
int EBCL_taskIncludeHandler(ebcl_Task_t *tgt, const char *val);

#endif /* __CONFHDL_H__ */
