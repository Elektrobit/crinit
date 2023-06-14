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

typedef int (*ebcl_FeatActivationFunc_t)(void);

typedef struct ebcl_OptFeatMap_t {
    const char *name;
    const ebcl_FeatActivationFunc_t af;
    const ebcl_GlobOptKey_t optkey;
} ebcl_OptFeatMap_t;

static int EBCL_activateSyslog(void) {
    crinitSetUseSyslog(true);
    return 0;
}

int EBCL_crinitFeatureHook(const char *sysFeatName) {
    if (sysFeatName == NULL) {
        crinitErrPrint("Input parameter must not be NULL.");
        return -1;
    }

    static const ebcl_OptFeatMap_t fmap[] = {
        {.name = "syslog", .af = EBCL_activateSyslog, .optkey = EBCL_GLOBOPT_USE_SYSLOG},
    };

    size_t n = sizeof(fmap) / sizeof(fmap[0]);
    for (size_t i = 0; i < n; i++) {
        if (strcmp(fmap[i].name, sysFeatName) == 0) {
            bool armed;
            if (EBCL_globOptGetBoolean(fmap[i].optkey, &armed) == -1) {
                crinitErrPrint("Could not get global setting for optional feature \'%s\'.", fmap[i].name);
                return -1;
            }
            return (armed) ? fmap[i].af() : 0;
        }
    }

    return 0;
}
