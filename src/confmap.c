/**
 * @file confmap.c
 * @brief Implementation file related to mapping configuration options and their string representations.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "confmap.h"

#include <stdlib.h>

#include "common.h"
#include "logio.h"

const ebcl_ConfigMapping_t EBCL_cfgMap[] = {
    {EBCL_TASK_CONFIG_COMMAND, EBCL_TASK_CONFIG_KEYSTR_COMMAND, true, false, EBCL_taskCfgCmdHandler},
    {EBCL_TASK_CONFIG_DEPENDS, EBCL_TASK_CONFIG_KEYSTR_DEPENDS, true, true, EBCL_taskCfgDepHandler},
    {EBCL_TASK_CONFIG_ENV_SET, EBCL_TASK_CONFIG_KEYSTR_ENV_SET, true, true, EBCL_taskCfgEnvHandler},
    {EBCL_TASK_CONFIG_INCLUDE, EBCL_TASK_CONFIG_KEYSTR_INCLUDE, true, false, EBCL_taskIncludeHandler},
    {EBCL_TASK_CONFIG_IOREDIR, EBCL_TASK_CONFIG_KEYSTR_IOREDIR, true, true, EBCL_taskCfgIoRedirHandler},
    {EBCL_TASK_CONFIG_NAME, EBCL_TASK_CONFIG_KEYSTR_NAME, false, false, EBCL_taskCfgNameHandler},
    {EBCL_TASK_CONFIG_PROVIDES, EBCL_TASK_CONFIG_KEYSTR_PROVIDES, true, false, EBCL_taskCfgPrvHandler},
    {EBCL_TASK_CONFIG_RESPAWN, EBCL_TASK_CONFIG_KEYSTR_RESPAWN, false, false, EBCL_taskCfgRespHandler},
    {EBCL_TASK_CONFIG_RESPAWN_RETRIES, EBCL_TASK_CONFIG_KEYSTR_RESPAWN_RETRIES, false, false,
     EBCL_taskCfgRespRetHandler},
};
const size_t EBCL_cfgMapSize = EBCL_numElements(EBCL_cfgMap);

static int EBCL_compareConfigMappings(const void *a, const void *b);

const ebcl_ConfigMapping_t *EBCL_findConfigMapping(const char *keyStr) {
    EBCL_nullCheck(NULL, keyStr == NULL);
    ebcl_ConfigMapping_t searchKey = {0, keyStr, 0, 0, NULL};
    return bsearch(&searchKey, EBCL_cfgMap, EBCL_cfgMapSize, sizeof(*EBCL_cfgMap), EBCL_compareConfigMappings);
}

static int EBCL_compareConfigMappings(const void *a, const void *b) {
    const ebcl_ConfigMapping_t *pKey = a, *pElem = b;
    return strcmp(pKey->configKey, pElem->configKey);
}
