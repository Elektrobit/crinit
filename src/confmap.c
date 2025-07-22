// SPDX-License-Identifier: MIT
/**
 * @file confmap.c
 * @brief Implementation file related to mapping configuration options and their string representations.
 */
#include "confmap.h"

#include <stdlib.h>

#include "common.h"
#include "logio.h"

const crinitConfigMapping_t crinitTaskCfgMap[] = {
#ifdef ENABLE_CAPABILITIES
    {CRINIT_CONFIG_CAP_CLEAR, CRINIT_CONFIG_KEYSTR_CAP_CLEAR, true, false, crinitCfgCapClearHandler},
    {CRINIT_CONFIG_CAP_SET, CRINIT_CONFIG_KEYSTR_CAP_SET, true, false, crinitCfgCapSetHandler},
#endif
#ifdef ENABLE_CGROUP
    {CRINIT_CONFIG_CGROUP_NAME, CRINIT_CONFIG_KEYSTR_CGROUP_NAME, false, false, crinitCfgCgroupNameHandler},
    {CRINIT_CONFIG_CGROUP_PARAMS, CRINIT_CONFIG_KEYSTR_CGROUP_PARAMS, true, false, crinitCfgCgroupParamsHandler},
#endif
    {CRINIT_CONFIG_COMMAND, CRINIT_CONFIG_KEYSTR_COMMAND, true, false, crinitCfgCmdHandler},
    {CRINIT_CONFIG_DEPENDS, CRINIT_CONFIG_KEYSTR_DEPENDS, true, true, crinitCfgDepHandler},
    {CRINIT_CONFIG_ENV_SET, CRINIT_CONFIG_KEYSTR_ENV_SET, true, true, crinitCfgEnvHandler},
    {CRINIT_CONFIG_FILTER_DEFINE, CRINIT_CONFIG_KEYSTR_FILTER_DEFINE, true, true, crinitCfgFilterHandler},
    {CRINIT_CONFIG_GROUP, CRINIT_CONFIG_KEYSTR_GROUP, true, false, crinitCfgGroupHandler},
    {CRINIT_CONFIG_INCLUDE, CRINIT_CONFIG_KEYSTR_INCLUDE, true, false, crinitTaskIncludeHandler},
    {CRINIT_CONFIG_IOREDIR, CRINIT_CONFIG_KEYSTR_IOREDIR, true, true, crinitCfgIoRedirHandler},
    {CRINIT_CONFIG_NAME, CRINIT_CONFIG_KEYSTR_NAME, false, false, crinitCfgNameHandler},
    {CRINIT_CONFIG_PROVIDES, CRINIT_CONFIG_KEYSTR_PROVIDES, true, false, crinitCfgPrvHandler},
    {CRINIT_CONFIG_RESPAWN, CRINIT_CONFIG_KEYSTR_RESPAWN, false, false, crinitCfgRespHandler},
    {CRINIT_CONFIG_RESPAWN_RETRIES, CRINIT_CONFIG_KEYSTR_RESPAWN_RETRIES, false, false, crinitCfgRespRetHandler},
    {CRINIT_CONFIG_STOP_COMMAND, CRINIT_CONFIG_KEYSTR_STOP_COMMAND, true, false, crinitCfgStopCmdHandler},
    {CRINIT_CONFIG_USER, CRINIT_CONFIG_KEYSTR_USER, false, false, crinitCfgUserHandler}};
const size_t crinitTaskCfgMapSize = crinitNumElements(crinitTaskCfgMap);

const crinitConfigMapping_t crinitSeriesCfgMap[] = {
    {CRINIT_CONFIG_DEBUG, CRINIT_CONFIG_KEYSTR_DEBUG, false, false, crinitCfgDebugHandler},
#ifdef ENABLE_CAPABILITIES
    {CRINIT_CONFIG_DEFAULTCAPS, CRINIT_CONFIG_KEYSTR_DEFAULTCAPS, true, false, crinitCfgDefaultCapsHandler},
#endif
    {CRINIT_CONFIG_ELOS_EVENT_POLL_INTERVAL, CRINIT_CONFIG_KEYSTR_ELOS_EVENT_POLL_INTERVAL, false, false,
     crinitCfgElosEventPollIntervalHandler},
    {CRINIT_CONFIG_ELOS_PORT, CRINIT_CONFIG_KEYSTR_ELOS_PORT, false, false, crinitCfgElosPortHandler},
    {CRINIT_CONFIG_ELOS_SERVER, CRINIT_CONFIG_KEYSTR_ELOS_SERVER, false, false, crinitCfgElosServerHandler},
    {CRINIT_CONFIG_ENV_SET, CRINIT_CONFIG_KEYSTR_ENV_SET, true, false, crinitCfgEnvHandler},
    {CRINIT_CONFIG_FILTER_DEFINE, CRINIT_CONFIG_KEYSTR_FILTER_DEFINE, true, false, crinitCfgFilterHandler},
    {CRINIT_CONFIG_INCLUDEDIR, CRINIT_CONFIG_KEYSTR_INCLDIR, false, false, crinitCfgInclDirHandler},
    {CRINIT_CONFIG_INCLUDE_SUFFIX, CRINIT_CONFIG_KEYSTR_INCL_SUFFIX, false, false, crinitCfgInclSuffixHandler},
    {CRINIT_CONFIG_LAUNCHER_CMD, CRINIT_CONFIG_KEYSTR_LAUNCHER_CMD, false, false, crinitCfgLauncherCmdHandler},
    {CRINIT_CONFIG_SHDGRACEP, CRINIT_CONFIG_KEYSTR_SHDGRACEP, false, false, crinitCfgShdGpHandler},
    {CRINIT_CONFIG_TASKDIR, CRINIT_CONFIG_KEYSTR_TASKDIR, false, false, crinitCfgTaskDirHandler},
    {CRINIT_CONFIG_TASKDIR_FOLLOW_SYMLINKS, CRINIT_CONFIG_KEYSTR_TASKDIR_SYMLINKS, false, false,
     crinitCfgTaskDirSlHandler},
    {CRINIT_CONFIG_TASKS, CRINIT_CONFIG_KEYSTR_TASKS, true, false, crinitCfgTasksHandler},
    {CRINIT_CONFIG_TASK_FILE_SUFFIX, CRINIT_CONFIG_KEYSTR_TASK_FILE_SUFFIX, false, false, crinitCfgTaskSuffixHandler},
    {CRINIT_CONFIG_USE_ELOS, CRINIT_CONFIG_KEYSTR_USE_ELOS, false, false, crinitCfgElosHandler},
    {CRINIT_CONFIG_USE_SYSLOG, CRINIT_CONFIG_KEYSTR_USE_SYSLOG, false, false, crinitCfgSyslogHandler}};
const size_t crinitSeriesCfgMapSize = crinitNumElements(crinitSeriesCfgMap);

const crinitConfigMapping_t crinitKCmdlineCfgMap[] = {
    {CRINIT_CONFIG_SIGKEYDIR, CRINIT_CONFIG_KEYSTR_SIGKEYDIR, false, false, crinitCfgSigKeyDirHandler},
    {CRINIT_CONFIG_SIGNATURES, CRINIT_CONFIG_KEYSTR_SIGNATURES, false, false, crinitCfgSignaturesHandler},
};
const size_t crinitKCmdlineCfgMapSize = crinitNumElements(crinitKCmdlineCfgMap);

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
