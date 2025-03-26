// SPDX-License-Identifier: MIT
/**
 * @file itest-stop-cmd-overrides.c
 * @brief Implementation of LD_PRELOAD overrides for the STOP_COMMAND robot test.
 */

#include <sys/reboot.h>
#include <sys/types.h>

#include "stdio.h"

// NOLINTNEXTLINE readability-identifier-naming Rationale: Naming given by standard library.
int reboot(int op) {
    fprintf(stderr, "Reboot called with operation ");
    switch (op) {
        case RB_POWER_OFF:
            fprintf(stderr, "RB_POWER_OFF.\n");
            break;
        case RB_AUTOBOOT:
            fprintf(stderr, "RB_AUTOBOOT.\n");
            break;
        default:
            fprintf(stderr, "%#08x.\n", op);
    }
    return 0;
}

// NOLINTNEXTLINE readability-identifier-naming Rationale: Naming given by standard library.
int kill(pid_t pid, int sig) {
    fprintf(stderr, "Requested to send signal %d to PID %d.\n", sig, pid);
    return 0;
}

// NOLINTNEXTLINE readability-identifier-naming Rationale: Naming given by standard library.
int umount2(const char *target, int flags) {
    fprintf(stderr, "Requested to unmount target '%s' with flags %#x.\n", target, flags);
    return 0;
}
