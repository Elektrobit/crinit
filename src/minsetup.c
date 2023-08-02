// SPDX-License-Identifier: MIT
/**
 * @file minsetup.c
 * @brief Implementation of minimal early system setup.
 */
#include "minsetup.h"

#include <stdbool.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "logio.h"

int crinitForkZombieReaper(void) {
    pid_t pid = fork();
    if (pid == -1) {
        crinitErrnoPrint("Could not fork to create a PID 1 zombie reaper parent.");
        return -1;
    }
    if (pid != 0) {  // parent process (must be PID 1)
        // lose in-/output
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        close(STDIN_FILENO);
        // becomes reaper of zombie processes
        while (true) {
            wait(NULL);
        }
    }
    // child returns
    return 0;
}

int crinitSetupSystemFs(void) {
    umask(0);

    if (mkdir("/dev", 0777) == -1) {
        if (errno != EEXIST) {
            crinitErrnoPrint("Could not create /dev directory: ");
            return -1;
        }
    }
    if (mount("none", "/dev", "devtmpfs", MS_NOEXEC | MS_NOSUID, NULL) == -1) {
        if (errno != EBUSY) {
            crinitErrnoPrint("Could not mount devtmpfs.");
            return -1;
        }
        crinitInfoPrint("/dev is already mounted. Skipping.");
    }

    if (mkdir("/proc", 0777) == -1) {
        if (errno != EEXIST) {
            crinitErrnoPrint("Could not create /proc directory: ");
            return -1;
        }
    }
    if (mount("none", "/proc", "proc", MS_NODEV | MS_NOEXEC | MS_NOSUID, NULL) == -1) {
        if (errno != EBUSY) {
            crinitErrnoPrint("Could not mount procfs.");
            return -1;
        }
        crinitInfoPrint("/proc is already mounted. Skipping.");
    }

    if (mkdir("/sys", 0777) == -1) {
        if (errno != EEXIST) {
            crinitErrnoPrint("Could not create /sys directory: ");
            return -1;
        }
    }
    if (mount("none", "/sys", "sysfs", MS_NODEV | MS_NOEXEC | MS_NOSUID, NULL) == -1) {
        if (errno != EBUSY) {
            crinitErrnoPrint("Could not mount sysfs.");
            return -1;
        }
        crinitInfoPrint("/sys is already mounted. Skipping.");
    }

    if (mkdir("/dev/pts", 0755) == -1) {
        if (errno != EEXIST) {
            crinitErrnoPrint("Could not create /dev/pts directory: ");
            return -1;
        }
    }
    if (mount("none", "/dev/pts", "devpts", MS_NOEXEC | MS_NOSUID, "mode=0620,ptmxmode=0666,gid=5") == -1) {
        if (errno != EBUSY) {
            crinitErrnoPrint("Could not mount devpts.");
            return -1;
        }
        crinitInfoPrint("/dev/pts is already mounted. Skipping.");
    }

    if (mkdir("/run", 0777) == -1) {
        if (errno != EEXIST) {
            crinitErrnoPrint("Could not create /run directory: ");
            return -1;
        }
    }
    if (mount("none", "/run", "tmpfs", MS_NODEV | MS_NOEXEC | MS_NOSUID, NULL) == -1) {
        if (errno != EBUSY) {
            crinitErrnoPrint("Could not mount tmpfs on /run.");
            return -1;
        }
        crinitInfoPrint("/run is already mounted. Skipping.");
    }

    umask(0022);
    return 0;
}
