/**
 * @file taskconfmap.h
 * @brief Definitions related to mapping configuration options to values inside an ebcl_Task_t.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __TASKCONFMAP_H__
#define __TASKCONFMAP_H__

#include "confhdl.h"

typedef struct ebcl_TaskConfigMapping_t {
    ebcl_TaskConfigs_t config;
    const char *configKey;
    bool arrayLike;
    bool includeSafe;
    ebcl_TaskConfigHandler_t cfgHandler;
} ebcl_TaskConfigMapping_t;

extern const ebcl_TaskConfigMapping_t EBCL_taskCfgMap[];
extern const size_t EBCL_taskCfgMapSize;

const ebcl_TaskConfigMapping_t *EBCL_findTaskConfigMapping(const char *keyStr);

#endif /* __TASKCONFMAP_H__ */
