// SPDX-License-Identifier: MIT
/**
 * @file optfeat.c
 * @brief Implementation of optional behavior dependent on available system features.
 */
#include "optfeat.h"

#include <string.h>

#include "common.h"
#include "elosio.h"
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

static int crinitElosioActivateCb(void *data) {
    CRINIT_PARAM_UNUSED(data);

    crinitElosioActivate((crinitTaskDB_t *)data, true);
    return 0;
}

static int crinitElosioTaskAddedCb(void *data) {
    crinitElosioTaskAdded((crinitTask_t *)data);
    return 0;
}

int crinitFeatureHook(const char *sysFeatName, crinitHookType_t type, void *data) {
    static const crinitOptFeatMap_t fmap[] = {
        {.name = "syslog",
         .type = START,
         .af = crinitActivateSyslog,
         .globMemberOffset = offsetof(crinitGlobOptStore_t, CRINIT_GLOBOPT_USE_SYSLOG)},
        {.name = CRINIT_ELOSIO_FEATURE_NAME,
         .type = START,
         .af = crinitElosioActivateCb,
         .globMemberOffset = offsetof(crinitGlobOptStore_t, CRINIT_GLOBOPT_USE_ELOS)},
        {.name = CRINIT_ELOSIO_FEATURE_NAME,
         .type = TASK_ADDED,
         .af = crinitElosioTaskAddedCb,
         .globMemberOffset = offsetof(crinitGlobOptStore_t, CRINIT_GLOBOPT_USE_ELOS)},
    };

    size_t n = sizeof(fmap) / sizeof(fmap[0]);
    for (size_t i = 0; i < n; i++) {
        if ((!sysFeatName || strcmp(fmap[i].name, sysFeatName) == 0) && fmap[i].type == type) {
            bool armed;
            crinitDbgInfoPrint("Executing feature hook for %s (%d) with %p.", fmap[i].name, type, data);
            if (crinitGlobOptGetBoolean(fmap[i].globMemberOffset, &armed) == -1) {
                crinitErrPrint("Could not get global setting for optional feature \'%s\'.", fmap[i].name);
                return -1;
            }
            return (armed) ? fmap[i].af(data) : 0;
        }
    }

    return 0;
}
