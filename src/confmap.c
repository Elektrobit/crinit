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

const crinitConfigMapping_t crinitTaskCfgMap[] = {
    {CRINIT_CONFIG_COMMAND, CRINIT_CONFIG_KEYSTR_COMMAND, true, false, crinitCfgCmdHandler},
    {CRINIT_CONFIG_DEPENDS, CRINIT_CONFIG_KEYSTR_DEPENDS, true, true, crinitCfgDepHandler},
    {CRINIT_CONFIG_ENV_SET, CRINIT_CONFIG_KEYSTR_ENV_SET, true, true, crinitCfgEnvHandler},
    {CRINIT_CONFIG_INCLUDE, CRINIT_CONFIG_KEYSTR_INCLUDE, true, false, crinitTaskIncludeHandler},
    {CRINIT_CONFIG_IOREDIR, CRINIT_CONFIG_KEYSTR_IOREDIR, true, true, crinitCfgIoRedirHandler},
    {CRINIT_CONFIG_NAME, CRINIT_CONFIG_KEYSTR_NAME, false, false, crinitCfgNameHandler},
    {CRINIT_CONFIG_PROVIDES, CRINIT_CONFIG_KEYSTR_PROVIDES, true, false, crinitCfgPrvHandler},
    {CRINIT_CONFIG_RESPAWN, CRINIT_CONFIG_KEYSTR_RESPAWN, false, false, crinitCfgRespHandler},
    {CRINIT_CONFIG_RESPAWN_RETRIES, CRINIT_CONFIG_KEYSTR_RESPAWN_RETRIES, false, false, crinitCfgRespRetHandler},
};
const size_t crinitTaskCfgMapSize = crinitNumElements(crinitTaskCfgMap);

const crinitConfigMapping_t crinitSeriesCfgMap[] = {
    {CRINIT_CONFIG_DEBUG, CRINIT_CONFIG_KEYSTR_DEBUG, false, false, crinitCfgDebugHandler},
    {CRINIT_CONFIG_ENV_SET, CRINIT_CONFIG_KEYSTR_ENV_SET, true, false, crinitCfgEnvHandler},
    {CRINIT_CONFIG_INCLUDEDIR, CRINIT_CONFIG_KEYSTR_INCLDIR, false, false, crinitCfgInclDirHandler},
    {CRINIT_CONFIG_INCLUDE_SUFFIX, CRINIT_CONFIG_KEYSTR_INCL_SUFFIX, false, false, crinitCfgInclSuffixHandler},
    {CRINIT_CONFIG_SHDGRACEP, CRINIT_CONFIG_KEYSTR_SHDGRACEP, false, false, crinitCfgShdGpHandler},
    {CRINIT_CONFIG_TASKDIR, CRINIT_CONFIG_KEYSTR_TASKDIR, false, false, crinitCfgTaskDirHandler},
    {CRINIT_CONFIG_TASKDIR_FOLLOW_SYMLINKS, CRINIT_CONFIG_KEYSTR_TASKDIR_SYMLINKS, false, false,
     crinitCfgTaskDirSlHandler},
    {CRINIT_CONFIG_TASKS, CRINIT_CONFIG_KEYSTR_TASKS, true, false, crinitCfgTasksHandler},
    {CRINIT_CONFIG_TASK_FILE_SUFFIX, CRINIT_CONFIG_KEYSTR_TASK_FILE_SUFFIX, false, false, crinitCfgTaskSuffixHandler},
    {CRINIT_CONFIG_USE_SYSLOG, CRINIT_CONFIG_KEYSTR_USE_SYSLOG, false, false, crinitCfgSyslogHandler},
};
const size_t crinitSeriesCfgMapSize = crinitNumElements(crinitSeriesCfgMap);

/** Comparison function between two crinitConfigMapping_t, for bsearch() **/
static int crinitCompareConfigMappings(const void *a, const void *b);

const crinitConfigMapping_t *crinitFindConfigMapping(const crinitConfigMapping_t *map, size_t mapSize,
                                                     const char *keyStr) {
    crinitNullCheck(NULL, map, keyStr);
    crinitConfigMapping_t searchKey = {0, keyStr, 0, 0, NULL};
    return bsearch(&searchKey, map, mapSize, sizeof(*map), crinitCompareConfigMappings);
}

static int crinitCompareConfigMappings(const void *a, const void *b) {
    const crinitConfigMapping_t *pKey = a, *pElem = b;
    return strcmp(pKey->configKey, pElem->configKey);
}
