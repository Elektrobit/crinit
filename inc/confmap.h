/**
 * @file confmap.h
 * @brief Definitions related to mapping configuration options and their string representations.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __CONFMAP_H__
#define __CONFMAP_H__

#include "confhdl.h"

typedef struct ebcl_ConfigMapping_t {
    ebcl_TaskConfigs_t config;
    const char *configKey;
    bool arrayLike;
    bool includeSafe;
    ebcl_ConfigHandler_t cfgHandler;
} ebcl_ConfigMapping_t;

extern const ebcl_ConfigMapping_t EBCL_cfgMap[];
extern const size_t EBCL_cfgMapSize;

const ebcl_ConfigMapping_t *EBCL_findConfigMapping(const char *keyStr);

#endif /* __CONFMAP_H__ */
