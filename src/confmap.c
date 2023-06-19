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

const crinitConfigMapping_t crinitCfgMap[] = {
    {CRINIT_CONFIG_COMMAND, CRINIT_CONFIG_KEYSTR_COMMAND, true, false, EBCL_taskCfgCmdHandler},
    {CRINIT_CONFIG_DEPENDS, CRINIT_CONFIG_KEYSTR_DEPENDS, true, true, EBCL_taskCfgDepHandler},
    {CRINIT_CONFIG_ENV_SET, CRINIT_CONFIG_KEYSTR_ENV_SET, true, true, EBCL_taskCfgEnvHandler},
    {CRINIT_CONFIG_INCLUDE, CRINIT_CONFIG_KEYSTR_INCLUDE, true, false, EBCL_taskIncludeHandler},
    {CRINIT_CONFIG_IOREDIR, CRINIT_CONFIG_KEYSTR_IOREDIR, true, true, EBCL_taskCfgIoRedirHandler},
    {CRINIT_CONFIG_NAME, CRINIT_CONFIG_KEYSTR_NAME, false, false, EBCL_taskCfgNameHandler},
    {CRINIT_CONFIG_PROVIDES, CRINIT_CONFIG_KEYSTR_PROVIDES, true, false, EBCL_taskCfgPrvHandler},
    {CRINIT_CONFIG_RESPAWN, CRINIT_CONFIG_KEYSTR_RESPAWN, false, false, EBCL_taskCfgRespHandler},
    {CRINIT_CONFIG_RESPAWN_RETRIES, CRINIT_CONFIG_KEYSTR_RESPAWN_RETRIES, false, false, EBCL_taskCfgRespRetHandler},
};
const size_t crinitCfgMapSize = crinitNumElements(crinitCfgMap);

/** Comparison function between two crinitConfigMapping_t, for bsearch() **/
static int EBCL_compareConfigMappings(const void *a, const void *b);

const crinitConfigMapping_t *crinitFindConfigMapping(const char *keyStr) {
    crinitNullCheck(NULL, keyStr);
    crinitConfigMapping_t searchKey = {0, keyStr, 0, 0, NULL};
    return bsearch(&searchKey, crinitCfgMap, crinitCfgMapSize, sizeof(*crinitCfgMap), EBCL_compareConfigMappings);
}

static int EBCL_compareConfigMappings(const void *a, const void *b) {
    const crinitConfigMapping_t *pKey = a, *pElem = b;
    return strcmp(pKey->configKey, pElem->configKey);
}
