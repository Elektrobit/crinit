// SPDX-License-Identifier: MIT
/**
 * @file capabilities.c
 * @brief Implementation of functions related to process capability handling.
 */
#include "capabilities.h"

#include <sys/capability.h>

#include "common.h"
#include "confconv.h"
#include "logio.h"

int crinitCapConvertToBitmask(uint64_t *bitmask, const char *capabilities) {
    crinitNullCheck(-1, bitmask, capabilities);
    int ret = 0;

    int confArrLen;
    char **parsedVal = crinitConfConvToStrArr(&confArrLen, capabilities, true);
    if (parsedVal == NULL) {
        crinitErrPrint("Could not convert capabilities list '%s' to string array.", capabilities);
        return -1;
    }
    *bitmask = 0;
    for (int i = 0; i < confArrLen; i++) {
        cap_value_t capability;
        if (cap_from_name(parsedVal[i], &capability) == 0) {
            *bitmask |= 1uL << capability;
            crinitDbgInfoPrint("Configure capability '%s' (%d).", parsedVal[i], capability);
        } else {
            crinitErrPrint("Could not convert capability '%s'.", parsedVal[i]);
            ret = -1;
            goto out;
        }
    }
    crinitDbgInfoPrint("Configured total set of capabilities %#lx.", *bitmask);

out:
    crinitFreeArgvArray(parsedVal);

    return ret;
}
