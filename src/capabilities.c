// SPDX-License-Identifier: MIT
/**
 * @file capabilities.c
 * @brief Implementation of functions related to process capability handling.
 */
#include "capabilities.h"

#include <linux/securebits.h>
#include <sys/capability.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "capabilities.h"
#include "common.h"
#include "confconv.h"
#include "logio.h"

/**
 * Gets capabilities of process specified by PID.
 *
 * @param out  Return pointer for capabilities. Note, that the Linux API defines cap_user_data_t as a pointer to
 *             struct __user_cap_header_struct. The given pointer needs to point to at least two elements.
 * @param pid  PID of the process from which to get the capabilities.
 *
 * @return 0 on success, -1 on error
 */
static int crinitProcCapGet(cap_user_data_t out, pid_t pid);

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

int crinitCapGetInheritable(pid_t pid, uint64_t *result) {
    crinitNullCheck(-1, result);
    struct __user_cap_data_struct capdata[2] = {0};

    if (crinitProcCapGet(capdata, pid) == -1) {
        crinitErrPrint("Could not get process capabilities.");
        return -1;
    }

    *result = capdata[0].inheritable;
    *result = *result | ((uint64_t)capdata[1].inheritable) << 32;

    return 0;
}

bool crinitCapIsCapsetEffective(cap_value_t cap, pid_t pid) {
    struct __user_cap_data_struct capdata[2] = {0};

    if (!CAP_IS_SUPPORTED(cap)) {
        crinitInfoPrint("Capability %d is not supported", cap);
        return false;
    }

    if (crinitProcCapGet(capdata, pid) == -1) {
        crinitErrPrint("Could not get process capabilities.");
        return false;
    }

    return (capdata[CAP_TO_INDEX(cap)].effective & CAP_TO_MASK(cap)) != 0;
}

int crinitCapSetAmbient(uint64_t capMask) {
    if ((capMask >> (CAP_LAST_CAP + 1)) > 0) {
        crinitErrPrint("Capability mask has capbility set which is out of range (Mask %#lx)", capMask);
        return -1;
    }

    for (int capIdx = 0; capIdx <= CAP_LAST_CAP; capIdx++) {
        if (!(capMask & (1uLL << capIdx))) {
            continue;
        }
        if (!CAP_IS_SUPPORTED(capIdx)) {
            crinitInfoPrint("Capability %d is not supported", capIdx);
            return -1;
        }

        if (prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, capIdx, 0, 0) == -1) {
            crinitErrPrint("Failed to set ambient capability.");
            return -1;
        }
    }

    return 0;
}

static int crinitSetCapStateInheritable(uint64_t capMask, cap_t capState) {
    if ((capMask >> (CAP_LAST_CAP + 1)) > 0) {
        crinitErrPrint("Capability mask has capbility set which is out of range (Mask %#lx)", capMask);
        return -1;
    }

    for (cap_value_t capIdx = 0; capIdx <= CAP_LAST_CAP; capIdx++) {
        if (!(capMask & (1uLL << capIdx))) {
            continue;
        }

        if (!CAP_IS_SUPPORTED(capIdx)) {
            crinitInfoPrint("Capability %d is not supported", capIdx);
            return -1;
        }

        if (cap_set_flag(capState, CAP_INHERITABLE, 1, &capIdx, CAP_SET) == -1) {
            crinitErrPrint("Failed to set capability flag %d", capIdx);
            return -1;
        }
    }
    return 0;
}

int crinitCapSetInheritable(uint64_t capMask) {
    crinitInfoPrint("Inheritable capabilities to be set %#lx", capMask);
    int ret = 0;

    cap_t capState = cap_get_proc();
    if (capState == NULL) {
        crinitErrPrint("Failed to get capability state");
        return -1;
    }

    if (cap_clear_flag(capState, CAP_INHERITABLE) == -1) {
        crinitErrPrint("Failed to clear inheritable capability state.");
        ret = -1;
        goto err_out;
    }

    if (crinitSetCapStateInheritable(capMask, capState) == -1) {
        crinitErrPrint("Failed to set inheritable capability state.");
        ret = -1;
        goto err_out;
    }

    if (cap_set_proc(capState) == -1) {
        crinitErrPrint("Failed to set capability state for capability mask %#lx", capMask);
        ret = -1;
        goto err_out;
    }

err_out:
    if (cap_free(capState) == -1) {
        crinitErrPrint("Failed to free capability resource");
    }
    return ret;
}

int crinitCapRetainPermitted() {
    int ret = prctl(PR_GET_SECUREBITS);
    if (ret == -1) {
        crinitErrPrint("Failed to get secure bits");
        return -1;
    }

    ret = prctl(PR_SET_SECUREBITS, ret | SECBIT_KEEP_CAPS);
    if (ret == -1) {
        crinitErrPrint("Failed to set secure bits flags SECBIT_KEEP_CAPS");
        return -1;
    }

    return 0;
}

static int crinitProcCapGet(cap_user_data_t out, pid_t pid) {
    crinitInfoPrint("Get capability for process %d.", pid);
    struct __user_cap_header_struct caphdr = {.version = _LINUX_CAPABILITY_VERSION_3, .pid = pid};
    if (syscall(SYS_capget, &caphdr, out) == -1) {
        crinitErrPrint("Could not get capabilities of process with PID %d.", pid);
        return -1;
    }
    return 0;
}
