/**
 * @file optfeat.c
 * @brief Implementation of optional behavior dependent on available system features.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "optfeat.h"

#include <string.h>

#include "globopt.h"
#include "logio.h"

typedef int (*crinitFeatActivationFunc_t)(void);

typedef struct crinitOptFeatMap_t {
    const char *name;
    const crinitFeatActivationFunc_t af;
    const crinitGlobOptKey_t optkey;
} crinitOptFeatMap_t;

static int crinitActivateSyslog(void) {
    crinitSetUseSyslog(true);
    return 0;
}

int crinitFeatureHook(const char *sysFeatName) {
    if (sysFeatName == NULL) {
        crinitErrPrint("Input parameter must not be NULL.");
        return -1;
    }

    static const crinitOptFeatMap_t fmap[] = {
        {.name = "syslog", .af = crinitActivateSyslog, .optkey = CRINIT_GLOBOPT_USE_SYSLOG},
    };

    size_t n = sizeof(fmap) / sizeof(fmap[0]);
    for (size_t i = 0; i < n; i++) {
        if (strcmp(fmap[i].name, sysFeatName) == 0) {
            bool armed;
            if (crinitGlobOptGetBoolean(fmap[i].optkey, &armed) == -1) {
                crinitErrPrint("Could not get global setting for optional feature \'%s\'.", fmap[i].name);
                return -1;
            }
            return (armed) ? fmap[i].af() : 0;
        }
    }

    return 0;
}
