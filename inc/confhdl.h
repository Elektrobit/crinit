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

typedef struct ebcl_CfgHdlContext_t {
    const char *val;
    size_t maxIdx[EBCL_TASK_CONFIGS_SIZE];
    size_t curIdx[EBCL_TASK_CONFIGS_SIZE];
} ebcl_CfgHdlCtx_t;

typedef int (*ebcl_ConfigHandler_t)(ebcl_Task_t *tgt, ebcl_CfgHdlCtx_t *handlerCtx);

int EBCL_taskCfgCmdHandler(ebcl_Task_t *tgt, ebcl_CfgHdlCtx_t *handlerCtx);
int EBCL_taskCfgDepHandler(ebcl_Task_t *tgt, ebcl_CfgHdlCtx_t *handlerCtx);
int EBCL_taskCfgEnvHandler(ebcl_Task_t *tgt, ebcl_CfgHdlCtx_t *handlerCtx);
int EBCL_taskCfgIoRedirHandler(ebcl_Task_t *tgt, ebcl_CfgHdlCtx_t *handlerCtx);
int EBCL_taskCfgNameHandler(ebcl_Task_t *tgt, ebcl_CfgHdlCtx_t *handlerCtx);
int EBCL_taskCfgPrvHandler(ebcl_Task_t *tgt, ebcl_CfgHdlCtx_t *handlerCtx);
int EBCL_taskCfgRespHandler(ebcl_Task_t *tgt, ebcl_CfgHdlCtx_t *handlerCtx);
int EBCL_taskCfgRespRetHandler(ebcl_Task_t *tgt, ebcl_CfgHdlCtx_t *handlerCtx);
int EBCL_taskIncludeHandler(ebcl_Task_t *tgt, ebcl_CfgHdlCtx_t *handlerCtx);

#endif /* __CONFHDL_H__ */
