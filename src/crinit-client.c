// SPDX-License-Identifier: MIT
/**
 * @file crinit-client.c
 * @brief Implementation of the crinit-client shared library.
 */
#include "crinit-client.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "globopt.h"
#include "logio.h"
#include "sockcom.h"

/**
 * Attribute macro for exported/visible functions, used together with -fvisibility=hidden to export only selected
 * functions
 */
#define CRINIT_LIB_EXPORTED __attribute__((visibility("default")))

#ifndef CRINIT_LIB_CONSTRUCTOR  // Guard so unit tests can define as empty.
/** Attribute macro for a function executed on loading of the shared library. **/
#define CRINIT_LIB_CONSTRUCTOR __attribute__((constructor))
#endif

#ifndef CRINIT_LIB_DESTRUCTOR  // Guard so unit tests can define as empty.
/** Attribute macro for a function executed on program exit if the shared library has been loaded before. **/
#define CRINIT_LIB_DESTRUCTOR __attribute__((destructor))
#endif

/** String to be used if no task name for sd_notify() is currently set. **/
#define CRINIT_ENV_NOTIFY_NAME_UNDEF "@undefined"

/** Holds the task name for sd_notify() **/
static const char *crinitNotifyName = CRINIT_ENV_NOTIFY_NAME_UNDEF;
/** Holds the path to the Crinit AF_UNIX socket file **/
static const char *crinitSockFile = CRINIT_SOCKFILE;

/**
 * Check if a response from Crinit is valid and/or an error.
 *
 * @param res      The response to check.
 * @param resCode  The expected response opcode.
 *
 * @return 0 if \a res is valid and indicates success, -1 if not
 */
static inline int crinitResponseCheck(const crinitRtimCmd_t *res, crinitRtimOp_t resCode);

/**
 * Library initialization function.
 *
 * Gets called on shared object loading through #CRINIT_LIB_CONSTRUCTOR attribute. Initializes options to default values
 * and tries to get the task name for sd_notify() from the environment if it is present.
 */
static CRINIT_LIB_CONSTRUCTOR void crinitLibInit(void) {
    bool v = false;
    crinitSetPrintPrefix("");
    crinitGlobOptSet(CRINIT_GLOBOPT_DEBUG, v);
    const char *envNotifyName = getenv(CRINIT_ENV_NOTIFY_NAME);
    if (envNotifyName != NULL) {
        crinitNotifyName = envNotifyName;
    } else {
        crinitNotifyName = CRINIT_ENV_NOTIFY_NAME_UNDEF;
    }
}

/**
 * Library cleanup function.
 *
 * Gets called on program end through #CRINIT_LIB_DESTRUCTOR attribute if the shared library was loaded. Frees global
 * option memory allocated as a consequence of crinitLibInit().
 */
static CRINIT_LIB_DESTRUCTOR void crinitLibDestroy(void) {
    crinitGlobOptDestroy();
}

CRINIT_LIB_EXPORTED int crinitClientSetVerbose(bool v) {
    return crinitGlobOptSet(CRINIT_GLOBOPT_DEBUG, v);
}

CRINIT_LIB_EXPORTED void crinitClientSetErrStream(FILE *errStream) {
    crinitSetErrStream(errStream);
}

CRINIT_LIB_EXPORTED void crinitClientSetInfoStream(FILE *infoStream) {
    crinitSetInfoStream(infoStream);
}

CRINIT_LIB_EXPORTED void crinitClientSetNotifyTaskName(const char *taskName) {
    if (taskName != NULL) {
        crinitNotifyName = taskName;
    }
}

CRINIT_LIB_EXPORTED void crinitClientSetSocketPath(const char *sockFile) {
    if (sockFile != NULL) {
        crinitSockFile = sockFile;
    }
}

CRINIT_LIB_EXPORTED const crinitVersion_t *crinitClientLibGetVersion(void) {
    return &crinitVersion;
}

CRINIT_LIB_EXPORTED int sd_notify(int unset_environment, const char *state) {  // NOLINT(readability-identifier-naming)
                                                                               // Rationale: Naming determined by
                                                                               // external API.
    if (state == NULL) {
        crinitErrPrint("State string must not be NULL");
        return -1;
    }

    if (unset_environment) {
        crinitErrPrint("SD_NOTIFY: unset_environment is unimplemented.");
    }

    crinitRtimCmd_t cmd, res;
    if (crinitBuildRtimCmd(&cmd, CRINIT_RTIMCMD_C_NOTIFY, 2, crinitNotifyName, state) == -1) {
        crinitErrPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }
    if (crinitXfer(crinitSockFile, &res, &cmd) == -1) {
        crinitDestroyRtimCmd(&cmd);
        crinitErrPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    crinitDestroyRtimCmd(&cmd);

    int ret = crinitResponseCheck(&res, CRINIT_RTIMCMD_R_NOTIFY);
    crinitDestroyRtimCmd(&res);
    return ret;
}

CRINIT_LIB_EXPORTED int sd_notifyf(                    // NOLINT(readability-identifier-naming)
    int unset_environment, const char *format, ...) {  // NOLINT(readability-identifier-naming)
                                                       // Rationale (both): Naming determined by external API.
    if (format == NULL) {
        crinitErrPrint("Format string must not be NULL");
        return -1;
    }

    va_list vargs, vargsCopy;
    va_start(vargs, format);
    va_copy(vargsCopy, vargs);
    size_t n = vsnprintf(NULL, 0, format, vargs) + 1;
    va_end(vargs);
    char *stateStr = malloc(n);
    if (stateStr == NULL) {
        crinitErrPrint("Could not allocate memory for SD_NOTIFY command string.");
        return -1;
    }
    vsnprintf(stateStr, n, format, vargsCopy);
    va_end(vargsCopy);

    int ret = sd_notify(unset_environment, stateStr);

    free(stateStr);
    return ret;
}

CRINIT_LIB_EXPORTED int crinitClientTaskAdd(const char *configFilePath, bool overwrite, const char *forceDeps) {
    if (configFilePath == NULL) {
        crinitErrPrint("Config file path must not be NULL");
        return -1;
    }

    crinitRtimCmd_t cmd, res;
    const char *overwrStr = (overwrite) ? "true" : "false";
    if (forceDeps == NULL) {
        forceDeps = "@unchanged";
    }

    if (strcmp(forceDeps, "") == 0) {
        forceDeps = "@empty";
    }

    if (crinitBuildRtimCmd(&cmd, CRINIT_RTIMCMD_C_ADDTASK, 3, configFilePath, overwrStr, forceDeps) == -1) {
        crinitErrPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (crinitXfer(crinitSockFile, &res, &cmd) == -1) {
        crinitDestroyRtimCmd(&cmd);
        crinitErrPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    crinitDestroyRtimCmd(&cmd);

    int ret = crinitResponseCheck(&res, CRINIT_RTIMCMD_R_ADDTASK);
    crinitDestroyRtimCmd(&res);
    return ret;
}

CRINIT_LIB_EXPORTED int crinitClientSeriesAdd(const char *seriesFilePath, bool overwriteTasks) {
    if (seriesFilePath == NULL) {
        crinitErrPrint("Series file path must not be NULL");
        return -1;
    }

    crinitRtimCmd_t cmd, res;
    const char *overwrStr = (overwriteTasks) ? "true" : "false";

    if (crinitBuildRtimCmd(&cmd, CRINIT_RTIMCMD_C_ADDSERIES, 2, seriesFilePath, overwrStr) == -1) {
        crinitErrPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (crinitXfer(crinitSockFile, &res, &cmd) == -1) {
        crinitDestroyRtimCmd(&cmd);
        crinitErrPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    crinitDestroyRtimCmd(&cmd);

    int ret = crinitResponseCheck(&res, CRINIT_RTIMCMD_R_ADDSERIES);
    crinitDestroyRtimCmd(&res);
    return ret;
}

CRINIT_LIB_EXPORTED int crinitClientTaskEnable(const char *taskName) {
    if (taskName == NULL) {
        crinitErrPrint("Task name must not be NULL");
        return -1;
    }

    crinitRtimCmd_t cmd, res;
    if (crinitBuildRtimCmd(&cmd, CRINIT_RTIMCMD_C_ENABLE, 1, taskName) == -1) {
        crinitErrPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (crinitXfer(crinitSockFile, &res, &cmd) == -1) {
        crinitDestroyRtimCmd(&cmd);
        crinitErrPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    crinitDestroyRtimCmd(&cmd);

    int ret = crinitResponseCheck(&res, CRINIT_RTIMCMD_R_ENABLE);
    crinitDestroyRtimCmd(&res);
    return ret;
}

CRINIT_LIB_EXPORTED int crinitClientTaskDisable(const char *taskName) {
    if (taskName == NULL) {
        crinitErrPrint("Task name must not be NULL");
        return -1;
    }
    crinitRtimCmd_t cmd, res;
    if (crinitBuildRtimCmd(&cmd, CRINIT_RTIMCMD_C_DISABLE, 1, taskName) == -1) {
        crinitErrPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (crinitXfer(crinitSockFile, &res, &cmd) == -1) {
        crinitDestroyRtimCmd(&cmd);
        crinitErrPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    crinitDestroyRtimCmd(&cmd);

    int ret = crinitResponseCheck(&res, CRINIT_RTIMCMD_R_DISABLE);
    crinitDestroyRtimCmd(&res);
    return ret;
}

CRINIT_LIB_EXPORTED int crinitClientTaskStop(const char *taskName) {
    if (taskName == NULL) {
        crinitErrPrint("Task name must not be NULL");
        return -1;
    }

    crinitRtimCmd_t cmd, res;
    if (crinitBuildRtimCmd(&cmd, CRINIT_RTIMCMD_C_STOP, 1, taskName) == -1) {
        crinitErrPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (crinitXfer(crinitSockFile, &res, &cmd) == -1) {
        crinitDestroyRtimCmd(&cmd);
        crinitErrPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    crinitDestroyRtimCmd(&cmd);

    int ret = crinitResponseCheck(&res, CRINIT_RTIMCMD_R_STOP);
    crinitDestroyRtimCmd(&res);
    return ret;
}

CRINIT_LIB_EXPORTED int crinitClientTaskKill(const char *taskName) {
    if (taskName == NULL) {
        crinitErrPrint("Task name must not be NULL");
        return -1;
    }

    crinitRtimCmd_t cmd, res;
    if (crinitBuildRtimCmd(&cmd, CRINIT_RTIMCMD_C_KILL, 1, taskName) == -1) {
        crinitErrPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (crinitXfer(crinitSockFile, &res, &cmd) == -1) {
        crinitDestroyRtimCmd(&cmd);
        crinitErrPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    crinitDestroyRtimCmd(&cmd);

    int ret = crinitResponseCheck(&res, CRINIT_RTIMCMD_R_KILL);
    crinitDestroyRtimCmd(&res);
    return ret;
}

CRINIT_LIB_EXPORTED int crinitClientTaskRestart(const char *taskName) {
    if (taskName == NULL) {
        crinitErrPrint("Task name must not be NULL");
        return -1;
    }

    crinitRtimCmd_t cmd, res;
    if (crinitBuildRtimCmd(&cmd, CRINIT_RTIMCMD_C_RESTART, 1, taskName) == -1) {
        crinitErrPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (crinitXfer(crinitSockFile, &res, &cmd) == -1) {
        crinitDestroyRtimCmd(&cmd);
        crinitErrPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    crinitDestroyRtimCmd(&cmd);

    int ret = crinitResponseCheck(&res, CRINIT_RTIMCMD_R_RESTART);
    crinitDestroyRtimCmd(&res);
    return ret;
}

CRINIT_LIB_EXPORTED int crinitClientTaskGetStatus(crinitTaskState_t *s, pid_t *pid, const char *taskName) {
    if (taskName == NULL || s == NULL) {
        crinitErrPrint("Pointer arguments must not be NULL");
        return -1;
    }

    crinitRtimCmd_t cmd, res;
    if (crinitBuildRtimCmd(&cmd, CRINIT_RTIMCMD_C_STATUS, 1, taskName) == -1) {
        crinitErrPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (crinitXfer(crinitSockFile, &res, &cmd) == -1) {
        crinitDestroyRtimCmd(&cmd);
        crinitErrPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    crinitDestroyRtimCmd(&cmd);

    if (crinitResponseCheck(&res, CRINIT_RTIMCMD_R_STATUS) == 0 && res.argc == 3) {
        *s = strtoul(res.args[1], NULL, 10);
        *pid = strtol(res.args[2], NULL, 10);
        crinitDestroyRtimCmd(&res);
        return 0;
    }
    crinitDestroyRtimCmd(&res);
    return -1;
}

CRINIT_LIB_EXPORTED int crinitClientGetTaskList(crinitTaskList_t **tlptr) {
    if (tlptr == NULL) {
        crinitErrPrint("Pointer arguments must not be NULL");
        return -1;
    }

    crinitRtimCmd_t cmd, res;
    if (crinitBuildRtimCmd(&cmd, CRINIT_RTIMCMD_C_TASKLIST, 0) == -1) {
        crinitErrPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (crinitXfer(crinitSockFile, &res, &cmd) == -1) {
        crinitDestroyRtimCmd(&cmd);
        crinitErrPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    crinitDestroyRtimCmd(&cmd);

    if (crinitResponseCheck(&res, CRINIT_RTIMCMD_R_TASKLIST) == -1) {
        crinitDestroyRtimCmd(&res);
        return -1;
    }

    int ret = 0;

    *tlptr = malloc(sizeof(crinitTaskList_t));
    if (*tlptr == NULL) {
        crinitErrPrint("Could not allocate memory for task list.");
        crinitDestroyRtimCmd(&res);
        return -1;
    }
    crinitTaskList_t *tl = *tlptr;
    tl->numTasks = 0;
    tl->tasks = malloc((res.argc - 1) * sizeof(*(tl->tasks)));
    if (tl->tasks == NULL) {
        crinitErrPrint("Could not allocate memory for task list entries.");
        ret = -1;
        goto fail_status;
    }

    for (size_t i = 0; i < res.argc - 1; i++) {
        const char *name = res.args[i + 1];
        pid_t pid = -1;
        crinitTaskState_t state = 0;

        if (crinitClientTaskGetStatus(&state, &pid, name) == -1) {
            crinitErrPrint("Querying status of task \'%s\' failed.", name);
            ret = -1;
            goto fail_status;
        }

        tl->tasks[i].name = strdup(name);
        if (tl->tasks[i].name == NULL) {
            crinitErrPrint("Could not allocate memory for task list entry name.");
            ret = -1;
            goto fail_status;
        }
        tl->tasks[i].pid = pid;
        tl->tasks[i].state = state;
        tl->numTasks++;
    }

    crinitDestroyRtimCmd(&res);
    return 0;

fail_status:
    crinitClientFreeTaskList(tl);
    crinitDestroyRtimCmd(&res);
    return ret;
}

CRINIT_LIB_EXPORTED void crinitClientFreeTaskList(crinitTaskList_t *tl) {
    if (tl == NULL) {
        return;
    }
    for (size_t i = 0; i < tl->numTasks; i++) {
        free(tl->tasks[i].name);
    }
    free(tl->tasks);
    free(tl);
}

CRINIT_LIB_EXPORTED int crinitClientShutdown(crinitShutdownCmd_t sCmd) {
    crinitRtimCmd_t cmd, res;
    char sCmdStr[2] = {0};
    snprintf(sCmdStr, 2, "%d", sCmd);
    if (crinitBuildRtimCmd(&cmd, CRINIT_RTIMCMD_C_SHUTDOWN, 1, sCmdStr) == -1) {
        crinitErrPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (crinitXfer(crinitSockFile, &res, &cmd) == -1) {
        crinitDestroyRtimCmd(&cmd);
        crinitErrPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    crinitDestroyRtimCmd(&cmd);

    int ret = crinitResponseCheck(&res, CRINIT_RTIMCMD_R_SHUTDOWN);
    crinitDestroyRtimCmd(&res);
    return ret;
}

CRINIT_LIB_EXPORTED int crinitClientGetVersion(crinitVersion_t *v) {
    if (v == NULL) {
        crinitErrPrint("Return pointer must not be NULL.");
        return -1;
    }
    crinitRtimCmd_t cmd, res;

    if (crinitBuildRtimCmd(&cmd, CRINIT_RTIMCMD_C_GETVER, 0) == -1) {
        crinitErrPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (crinitXfer(crinitSockFile, &res, &cmd) == -1) {
        crinitDestroyRtimCmd(&cmd);
        crinitErrPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    crinitDestroyRtimCmd(&cmd);

    int ret = crinitResponseCheck(&res, CRINIT_RTIMCMD_R_GETVER);
    if (ret == 0) {
        if (res.argc != 5) {
            crinitErrPrint("Got unexpected response length from Crinit.");
            crinitDestroyRtimCmd(&res);
            return -1;
        }

        char *endPtr = NULL;

        v->major = (uint8_t)strtoul(res.args[1], &endPtr, 10);
        if (endPtr == res.args[1]) {
            crinitErrPrint("Could not convert major version number to integer.");
            crinitDestroyRtimCmd(&res);
            return -1;
        }

        v->minor = (uint8_t)strtoul(res.args[2], &endPtr, 10);
        if (endPtr == res.args[2]) {
            crinitErrPrint("Could not convert minor version number to integer.");
            crinitDestroyRtimCmd(&res);
            return -1;
        }

        v->micro = (uint8_t)strtoul(res.args[3], &endPtr, 10);
        if (endPtr == res.args[3]) {
            crinitErrPrint("Could not convert micro version number to integer.");
            crinitDestroyRtimCmd(&res);
            return -1;
        }

        strncpy(v->git, res.args[4], sizeof(v->git) - 1);
        v->git[sizeof(v->git) - 1] = '\0';
    }

    crinitDestroyRtimCmd(&res);
    return ret;
}

static inline int crinitResponseCheck(const crinitRtimCmd_t *res, crinitRtimOp_t resCode) {
    if (res == NULL) {
        crinitErrPrint("Pointer arguments must not be NULL.");
        return -1;
    }
    if (res->op != resCode) {
        crinitErrPrint("Got unexpected response opcode from Crinit: %d", res->op);
        return -1;
    }

    if (res->argc == 0) {
        crinitErrPrint("Got unexpected response length from Crinit.");
        return -1;
    }

    if (strcmp(res->args[0], CRINIT_RTIMCMD_RES_OK) == 0) {
        return 0;
    }

    crinitErrPrint("Crinit responded with an error message.");
    if (res->argc >= 2) {
        crinitErrPrint("Message from Crinit: \'%s\'", res->args[1]);
    }

    return -1;
}
