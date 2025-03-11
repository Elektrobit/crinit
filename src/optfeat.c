// SPDX-License-Identifier: MIT
/**
 * @file optfeat.c
 * @brief Implementation of optional behavior dependent on available system features.
 */
#include "optfeat.h"

#include <string.h>

#include "common.h"
#ifdef ENABLE_ELOS
#include "elosdep.h"
#include "eloslog.h"
#endif
#include "globopt.h"
#include "logio.h"
#include "taskdb.h"

typedef int (*crinitFeatActivationFunc_t)(void *data);

typedef struct crinitOptFeatMap_t {
    const char *name;
    const crinitHookType_t type;
    const crinitFeatActivationFunc_t af;
    const size_t globMemberOffset;
} crinitOptFeatMap_t;

static int crinitActivateSyslog(void *data) {
    CRINIT_PARAM_UNUSED(data);

    crinitSetUseSyslog(true);
    return 0;
}

#ifdef ENABLE_ELOS
static int crinitElosdepActivateCb(void *data) {
    return crinitElosdepActivate((crinitTaskDB_t *)data, true);
}

static int crinitElosdepDeactivateCb(void *data) {
    return crinitElosdepActivate((crinitTaskDB_t *)data, false);
}

static int crinitElosdepTaskAddedCb(void *data) {
    return crinitElosdepTaskAdded((crinitTask_t *)data);
}

static int crinitEloslogInitCb(void *data) {
    CRINIT_PARAM_UNUSED(data);

    return crinitEloslogInit();
}

static int crinitEloslogActivateCb(void *data) {
    CRINIT_PARAM_UNUSED(data);

    return crinitEloslogActivate(true);
}

static int crinitEloslogDeactivateCb(void *data) {
    CRINIT_PARAM_UNUSED(data);

    return crinitEloslogActivate(false);
}
#endif

int crinitFeatureHook(const char *sysFeatName, crinitHookType_t type, void *data) {
    static const crinitOptFeatMap_t fmap[] = {
        {.name = "syslog",
         .type = CRINIT_HOOK_START,
         .af = crinitActivateSyslog,
         .globMemberOffset = offsetof(crinitGlobOptStore_t, CRINIT_GLOBOPT_USE_SYSLOG)},
#ifdef ENABLE_ELOS
        {.name = CRINIT_ELOSDEP_FEATURE_NAME,
         .type = CRINIT_HOOK_START,
         .af = crinitElosdepActivateCb,
         .globMemberOffset = offsetof(crinitGlobOptStore_t, CRINIT_GLOBOPT_USE_ELOS)},
        {.name = CRINIT_ELOSDEP_FEATURE_NAME,
         .type = CRINIT_HOOK_STOP,
         .af = crinitElosdepDeactivateCb,
         .globMemberOffset = offsetof(crinitGlobOptStore_t, CRINIT_GLOBOPT_USE_ELOS)},
        {.name = CRINIT_ELOSDEP_FEATURE_NAME,
         .type = CRINIT_HOOK_TASK_ADDED,
         .af = crinitElosdepTaskAddedCb,
         .globMemberOffset = offsetof(crinitGlobOptStore_t, CRINIT_GLOBOPT_USE_ELOS)},
        {.name = CRINIT_ELOSLOG_FEATURE_NAME,
         .type = CRINIT_HOOK_INIT,
         .af = crinitEloslogInitCb,
         .globMemberOffset = offsetof(crinitGlobOptStore_t, CRINIT_GLOBOPT_USE_ELOS)},
        {.name = CRINIT_ELOSLOG_FEATURE_NAME,
         .type = CRINIT_HOOK_START,
         .af = crinitEloslogActivateCb,
         .globMemberOffset = offsetof(crinitGlobOptStore_t, CRINIT_GLOBOPT_USE_ELOS)},
        {.name = CRINIT_ELOSLOG_FEATURE_NAME,
         .type = CRINIT_HOOK_STOP,
         .af = crinitEloslogDeactivateCb,
         .globMemberOffset = offsetof(crinitGlobOptStore_t, CRINIT_GLOBOPT_USE_ELOS)},
#endif
    };

    crinitDbgInfoPrint("Searching feature hooks for %s (%d) with %p.", sysFeatName, type, data);

    int res = 0;

    size_t n = sizeof(fmap) / sizeof(fmap[0]);
    for (size_t i = 0; i < n; i++) {
        if ((!sysFeatName || strcmp(fmap[i].name, sysFeatName) == 0) && fmap[i].type == type) {
            bool armed;
            crinitDbgInfoPrint("Executing feature hook for %s (%d) with %p.", fmap[i].name, type, data);
            if (crinitGlobOptGetBoolean(fmap[i].globMemberOffset, &armed) == -1) {
                crinitErrPrint("Could not get global setting for optional feature \'%s\'.", fmap[i].name);
                return -1;
            }
            res |= (armed) ? fmap[i].af(data) : 0;
        }
    }

    return res;
}
