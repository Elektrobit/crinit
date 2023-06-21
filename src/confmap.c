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
    {CRINIT_CONFIG_COMMAND, CRINIT_CONFIG_KEYSTR_COMMAND, true, false, crinitTaskCfgCmdHandler},
    {CRINIT_CONFIG_DEPENDS, CRINIT_CONFIG_KEYSTR_DEPENDS, true, true, crinitTaskCfgDepHandler},
    {CRINIT_CONFIG_ENV_SET, CRINIT_CONFIG_KEYSTR_ENV_SET, true, true, crinitTaskCfgEnvHandler},
    {CRINIT_CONFIG_INCLUDE, CRINIT_CONFIG_KEYSTR_INCLUDE, true, false, crinitTaskIncludeHandler},
    {CRINIT_CONFIG_IOREDIR, CRINIT_CONFIG_KEYSTR_IOREDIR, true, true, crinitTaskCfgIoRedirHandler},
    {CRINIT_CONFIG_NAME, CRINIT_CONFIG_KEYSTR_NAME, false, false, crinitTaskCfgNameHandler},
    {CRINIT_CONFIG_PROVIDES, CRINIT_CONFIG_KEYSTR_PROVIDES, true, false, crinitTaskCfgPrvHandler},
    {CRINIT_CONFIG_RESPAWN, CRINIT_CONFIG_KEYSTR_RESPAWN, false, false, crinitTaskCfgRespHandler},
    {CRINIT_CONFIG_RESPAWN_RETRIES, CRINIT_CONFIG_KEYSTR_RESPAWN_RETRIES, false, false, crinitTaskCfgRespRetHandler},
};
const size_t crinitCfgMapSize = crinitNumElements(crinitCfgMap);

/** Comparison function between two crinitConfigMapping_t, for bsearch() **/
static int crinitCompareConfigMappings(const void *a, const void *b);

const crinitConfigMapping_t *crinitFindConfigMapping(const char *keyStr) {
    crinitNullCheck(NULL, keyStr);
    crinitConfigMapping_t searchKey = {0, keyStr, 0, 0, NULL};
    return bsearch(&searchKey, crinitCfgMap, crinitCfgMapSize, sizeof(*crinitCfgMap), crinitCompareConfigMappings);
}

static int crinitCompareConfigMappings(const void *a, const void *b) {
    const crinitConfigMapping_t *pKey = a, *pElem = b;
    return strcmp(pKey->configKey, pElem->configKey);
}
