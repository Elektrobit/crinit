/**
 * @file crinit-client.c
 * @brief Implementation of the crinit-client shared library.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
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
#define EBCL_LIB_EXPORTED __attribute__((visibility("default")))

#ifndef EBCL_LIB_CONSTRUCTOR  // Guard so unit tests can define as empty.
/** Attribute macro for a function executed on loading of the shared library. **/
#define EBCL_LIB_CONSTRUCTOR __attribute__((constructor))
#endif

#ifndef EBCL_LIB_DESTRUCTOR  // Guard so unit tests can define as empty.
/** Attribute macro for a function executed on program exit if the shared library has been loaded before. **/
#define EBCL_LIB_DESTRUCTOR __attribute__((destructor))
#endif

/** String to be used if no task name for sd_notify() is currently set. **/
#define EBCL_CRINIT_ENV_NOTIFY_NAME_UNDEF "@undefined"

/** Holds the task name for sd_notify() **/
static const char *EBCL_notifyName = EBCL_CRINIT_ENV_NOTIFY_NAME_UNDEF;
/** Holds the path to the Crinit AF_UNIX socket file **/
static const char *EBCL_crinitSockFile = EBCL_CRINIT_SOCKFILE;

/**
 * Check if a response from Crinit is valid and/or an error.
 *
 * @param res      The response to check.
 * @param resCode  The expected response opcode.
 *
 * @return 0 if \a res is valid and indicates success, -1 if not
 */
static inline int EBCL_crinitResponseCheck(const ebcl_RtimCmd_t *res, ebcl_RtimOp_t resCode);

/**
 * Library initialization function.
 *
 * Gets called on shared object loading through #EBCL_LIB_CONSTRUCTOR attribute. Initializes options to default values
 * and tries to get the task name for sd_notify() from the environment if it is present.
 */
static EBCL_LIB_CONSTRUCTOR void EBCL_libInit(void) {
    bool v = false;
    EBCL_setPrintPrefix("");
    EBCL_globOptSetBoolean(EBCL_GLOBOPT_DEBUG, &v);
    const char *envNotifyName = getenv(EBCL_CRINIT_ENV_NOTIFY_NAME);
    if (envNotifyName != NULL) {
        EBCL_notifyName = envNotifyName;
    } else {
        EBCL_notifyName = EBCL_CRINIT_ENV_NOTIFY_NAME_UNDEF;
    }
}

/**
 * Library cleanup function.
 *
 * Gets called on program end through #EBCL_LIB_DESTRUCTOR attribute if the shared library was loaded. Frees global
 * option memory allocated as a consequence of EBCL_libInit().
 */
static EBCL_LIB_DESTRUCTOR void EBCL_libDestroy(void) {
    EBCL_globOptDestroy();
}

EBCL_LIB_EXPORTED int EBCL_crinitSetVerbose(bool v) {
    return EBCL_globOptSetBoolean(EBCL_GLOBOPT_DEBUG, &v);
}

EBCL_LIB_EXPORTED void EBCL_crinitSetErrStream(FILE *errStream) {
    EBCL_setErrStream(errStream);
}

EBCL_LIB_EXPORTED void EBCL_crinitSetInfoStream(FILE *infoStream) {
    EBCL_setInfoStream(infoStream);
}

EBCL_LIB_EXPORTED void EBCL_crinitSetNotifyTaskName(const char *taskName) {
    if (taskName != NULL) {
        EBCL_notifyName = taskName;
    }
}

EBCL_LIB_EXPORTED void EBCL_crinitSetSocketPath(const char *sockFile) {
    if (sockFile != NULL) {
        EBCL_crinitSockFile = sockFile;
    }
}

EBCL_LIB_EXPORTED const ebcl_Version_t *EBCL_crinitLibGetVersion(void) {
    return &EBCL_crinitVersion;
}

EBCL_LIB_EXPORTED int sd_notify(int unset_environment, const char *state) {  // NOLINT(readability-identifier-naming)
                                                                             // Rationale: Naming determined by
                                                                             // external API.
    if (state == NULL) {
        EBCL_errPrint("State string must not be NULL");
        return -1;
    }

    if (unset_environment) {
        EBCL_errPrint("SD_NOTIFY: unset_environment is unimplemented.");
    }

    ebcl_RtimCmd_t cmd, res;
    if (EBCL_buildRtimCmd(&cmd, EBCL_RTIMCMD_C_NOTIFY, 2, EBCL_notifyName, state) == -1) {
        EBCL_errPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }
    if (EBCL_crinitXfer(EBCL_crinitSockFile, &res, &cmd) == -1) {
        EBCL_destroyRtimCmd(&cmd);
        EBCL_errPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    EBCL_destroyRtimCmd(&cmd);

    int ret = EBCL_crinitResponseCheck(&res, EBCL_RTIMCMD_R_NOTIFY);
    EBCL_destroyRtimCmd(&res);
    return ret;
}

EBCL_LIB_EXPORTED int sd_notifyf(                      // NOLINT(readability-identifier-naming)
    int unset_environment, const char *format, ...) {  // NOLINT(readability-identifier-naming)
                                                       // Rationale (both): Naming determined by external API.
    if (format == NULL) {
        EBCL_errPrint("Format string must not be NULL");
        return -1;
    }

    va_list vargs, vargsCopy;
    va_start(vargs, format);
    va_copy(vargsCopy, vargs);
    size_t n = vsnprintf(NULL, 0, format, vargs) + 1;
    va_end(vargs);
    char *stateStr = malloc(n);
    if (stateStr == NULL) {
        EBCL_errPrint("Could not allocate memory for SD_NOTIFY command string.");
        return -1;
    }
    vsnprintf(stateStr, n, format, vargsCopy);
    va_end(vargsCopy);

    int ret = sd_notify(unset_environment, stateStr);

    free(stateStr);
    return ret;
}

EBCL_LIB_EXPORTED int EBCL_crinitTaskAdd(const char *configFilePath, bool overwrite, const char *forceDeps) {
    if (configFilePath == NULL) {
        EBCL_errPrint("Config file path must not be NULL");
        return -1;
    }

    ebcl_RtimCmd_t cmd, res;
    const char *overwrStr = (overwrite) ? "true" : "false";
    if (forceDeps == NULL) {
        forceDeps = "@unchanged";
    }

    if (strcmp(forceDeps, "") == 0) {
        forceDeps = "@empty";
    }

    if (EBCL_buildRtimCmd(&cmd, EBCL_RTIMCMD_C_ADDTASK, 3, configFilePath, overwrStr, forceDeps) == -1) {
        EBCL_errPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (EBCL_crinitXfer(EBCL_crinitSockFile, &res, &cmd) == -1) {
        EBCL_destroyRtimCmd(&cmd);
        EBCL_errPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    EBCL_destroyRtimCmd(&cmd);

    int ret = EBCL_crinitResponseCheck(&res, EBCL_RTIMCMD_R_ADDTASK);
    EBCL_destroyRtimCmd(&res);
    return ret;
}

EBCL_LIB_EXPORTED int EBCL_crinitSeriesAdd(const char *seriesFilePath, bool overwriteTasks) {
    if (seriesFilePath == NULL) {
        EBCL_errPrint("Series file path must not be NULL");
        return -1;
    }

    ebcl_RtimCmd_t cmd, res;
    const char *overwrStr = (overwriteTasks) ? "true" : "false";

    if (EBCL_buildRtimCmd(&cmd, EBCL_RTIMCMD_C_ADDSERIES, 2, seriesFilePath, overwrStr) == -1) {
        EBCL_errPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (EBCL_crinitXfer(EBCL_crinitSockFile, &res, &cmd) == -1) {
        EBCL_destroyRtimCmd(&cmd);
        EBCL_errPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    EBCL_destroyRtimCmd(&cmd);

    int ret = EBCL_crinitResponseCheck(&res, EBCL_RTIMCMD_R_ADDSERIES);
    EBCL_destroyRtimCmd(&res);
    return ret;
}

EBCL_LIB_EXPORTED int EBCL_crinitTaskEnable(const char *taskName) {
    if (taskName == NULL) {
        EBCL_errPrint("Task name must not be NULL");
        return -1;
    }

    ebcl_RtimCmd_t cmd, res;
    if (EBCL_buildRtimCmd(&cmd, EBCL_RTIMCMD_C_ENABLE, 1, taskName) == -1) {
        EBCL_errPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (EBCL_crinitXfer(EBCL_crinitSockFile, &res, &cmd) == -1) {
        EBCL_destroyRtimCmd(&cmd);
        EBCL_errPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    EBCL_destroyRtimCmd(&cmd);

    int ret = EBCL_crinitResponseCheck(&res, EBCL_RTIMCMD_R_ENABLE);
    EBCL_destroyRtimCmd(&res);
    return ret;
}

EBCL_LIB_EXPORTED int EBCL_crinitTaskDisable(const char *taskName) {
    if (taskName == NULL) {
        EBCL_errPrint("Task name must not be NULL");
        return -1;
    }
    ebcl_RtimCmd_t cmd, res;
    if (EBCL_buildRtimCmd(&cmd, EBCL_RTIMCMD_C_DISABLE, 1, taskName) == -1) {
        EBCL_errPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (EBCL_crinitXfer(EBCL_crinitSockFile, &res, &cmd) == -1) {
        EBCL_destroyRtimCmd(&cmd);
        EBCL_errPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    EBCL_destroyRtimCmd(&cmd);

    int ret = EBCL_crinitResponseCheck(&res, EBCL_RTIMCMD_R_DISABLE);
    EBCL_destroyRtimCmd(&res);
    return ret;
}

EBCL_LIB_EXPORTED int EBCL_crinitTaskStop(const char *taskName) {
    if (taskName == NULL) {
        EBCL_errPrint("Task name must not be NULL");
        return -1;
    }

    ebcl_RtimCmd_t cmd, res;
    if (EBCL_buildRtimCmd(&cmd, EBCL_RTIMCMD_C_STOP, 1, taskName) == -1) {
        EBCL_errPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (EBCL_crinitXfer(EBCL_crinitSockFile, &res, &cmd) == -1) {
        EBCL_destroyRtimCmd(&cmd);
        EBCL_errPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    EBCL_destroyRtimCmd(&cmd);

    int ret = EBCL_crinitResponseCheck(&res, EBCL_RTIMCMD_R_STOP);
    EBCL_destroyRtimCmd(&res);
    return ret;
}

EBCL_LIB_EXPORTED int EBCL_crinitTaskKill(const char *taskName) {
    if (taskName == NULL) {
        EBCL_errPrint("Task name must not be NULL");
        return -1;
    }

    ebcl_RtimCmd_t cmd, res;
    if (EBCL_buildRtimCmd(&cmd, EBCL_RTIMCMD_C_KILL, 1, taskName) == -1) {
        EBCL_errPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (EBCL_crinitXfer(EBCL_crinitSockFile, &res, &cmd) == -1) {
        EBCL_destroyRtimCmd(&cmd);
        EBCL_errPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    EBCL_destroyRtimCmd(&cmd);

    int ret = EBCL_crinitResponseCheck(&res, EBCL_RTIMCMD_R_KILL);
    EBCL_destroyRtimCmd(&res);
    return ret;
}

EBCL_LIB_EXPORTED int EBCL_crinitTaskRestart(const char *taskName) {
    if (taskName == NULL) {
        EBCL_errPrint("Task name must not be NULL");
        return -1;
    }

    ebcl_RtimCmd_t cmd, res;
    if (EBCL_buildRtimCmd(&cmd, EBCL_RTIMCMD_C_RESTART, 1, taskName) == -1) {
        EBCL_errPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (EBCL_crinitXfer(EBCL_crinitSockFile, &res, &cmd) == -1) {
        EBCL_destroyRtimCmd(&cmd);
        EBCL_errPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    EBCL_destroyRtimCmd(&cmd);

    int ret = EBCL_crinitResponseCheck(&res, EBCL_RTIMCMD_R_RESTART);
    EBCL_destroyRtimCmd(&res);
    return ret;
}

EBCL_LIB_EXPORTED int EBCL_crinitTaskGetStatus(ebcl_TaskState_t *s, pid_t *pid, const char *taskName) {
    if (taskName == NULL || s == NULL) {
        EBCL_errPrint("Pointer arguments must not be NULL");
        return -1;
    }

    ebcl_RtimCmd_t cmd, res;
    if (EBCL_buildRtimCmd(&cmd, EBCL_RTIMCMD_C_STATUS, 1, taskName) == -1) {
        EBCL_errPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (EBCL_crinitXfer(EBCL_crinitSockFile, &res, &cmd) == -1) {
        EBCL_destroyRtimCmd(&cmd);
        EBCL_errPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    EBCL_destroyRtimCmd(&cmd);

    if (EBCL_crinitResponseCheck(&res, EBCL_RTIMCMD_R_STATUS) == 0 && res.argc == 3) {
        *s = strtoul(res.args[1], NULL, 10);
        *pid = strtol(res.args[2], NULL, 10);
        EBCL_destroyRtimCmd(&res);
        return 0;
    }
    EBCL_destroyRtimCmd(&res);
    return -1;
}

EBCL_LIB_EXPORTED int EBCL_crinitGetTaskList(ebcl_TaskList_t **tlptr) {
    if (tlptr == NULL) {
        EBCL_errPrint("Pointer arguments must not be NULL");
        return -1;
    }

    ebcl_RtimCmd_t cmd, res;
    if (EBCL_buildRtimCmd(&cmd, EBCL_RTIMCMD_C_TASKLIST, 0) == -1) {
        EBCL_errPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (EBCL_crinitXfer(EBCL_crinitSockFile, &res, &cmd) == -1) {
        EBCL_destroyRtimCmd(&cmd);
        EBCL_errPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    EBCL_destroyRtimCmd(&cmd);

    if (EBCL_crinitResponseCheck(&res, EBCL_RTIMCMD_R_TASKLIST) == -1) {
        EBCL_destroyRtimCmd(&res);
        return -1;
    }

    int ret = 0;

    *tlptr = malloc(sizeof(ebcl_TaskList_t));
    if (*tlptr == NULL) {
        EBCL_errPrint("Could not allocate memory for task list.");
        EBCL_destroyRtimCmd(&res);
        return -1;
    }
    ebcl_TaskList_t *tl = *tlptr;
    tl->numTasks = 0;
    tl->tasks = malloc((res.argc - 1) * sizeof(*(tl->tasks)));
    if (tl->tasks == NULL) {
        EBCL_errPrint("Could not allocate memory for task list entries.");
        ret = -1;
        goto fail_status;
    }

    for (size_t i = 0; i < res.argc - 1; i++) {
        const char *name = res.args[i + 1];
        pid_t pid = -1;
        ebcl_TaskState_t state = 0;

        if (EBCL_crinitTaskGetStatus(&state, &pid, name) == -1) {
            EBCL_errPrint("Querying status of task \'%s\' failed.", name);
            ret = -1;
            goto fail_status;
        }

        tl->tasks[i].name = strdup(name);
        if (tl->tasks[i].name == NULL) {
            EBCL_errPrint("Could not allocate memory for task list entry name.");
            ret = -1;
            goto fail_status;
        }
        tl->tasks[i].pid = pid;
        tl->tasks[i].state = state;
        tl->numTasks++;
    }

    EBCL_destroyRtimCmd(&res);
    return 0;

fail_status:
    EBCL_crinitFreeTaskList(tl);
    EBCL_destroyRtimCmd(&res);
    return ret;
}

EBCL_LIB_EXPORTED void EBCL_crinitFreeTaskList(ebcl_TaskList_t *tl) {
    if (tl == NULL) {
        return;
    }
    for (size_t i = 0; i < tl->numTasks; i++) {
        free(tl->tasks[i].name);
    }
    free(tl->tasks);
    free(tl);
}

EBCL_LIB_EXPORTED int EBCL_crinitShutdown(int shutdownCmd) {
    ebcl_RtimCmd_t cmd, res;
    char shdCmdStr[16] = {0};
    snprintf(shdCmdStr, 16, "0x%x", shutdownCmd);
    if (EBCL_buildRtimCmd(&cmd, EBCL_RTIMCMD_C_SHUTDOWN, 1, shdCmdStr) == -1) {
        EBCL_errPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (EBCL_crinitXfer(EBCL_crinitSockFile, &res, &cmd) == -1) {
        EBCL_destroyRtimCmd(&cmd);
        EBCL_errPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    EBCL_destroyRtimCmd(&cmd);

    int ret = EBCL_crinitResponseCheck(&res, EBCL_RTIMCMD_R_SHUTDOWN);
    EBCL_destroyRtimCmd(&res);
    return ret;
}

EBCL_LIB_EXPORTED int EBCL_crinitGetVersion(ebcl_Version_t *v) {
    if (v == NULL) {
        EBCL_errPrint("Return pointer must not be NULL.");
        return -1;
    }
    ebcl_RtimCmd_t cmd, res;

    if (EBCL_buildRtimCmd(&cmd, EBCL_RTIMCMD_C_GETVER, 0) == -1) {
        EBCL_errPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (EBCL_crinitXfer(EBCL_crinitSockFile, &res, &cmd) == -1) {
        EBCL_destroyRtimCmd(&cmd);
        EBCL_errPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    EBCL_destroyRtimCmd(&cmd);

    int ret = EBCL_crinitResponseCheck(&res, EBCL_RTIMCMD_R_GETVER);
    if (ret == 0) {
        if (res.argc != 5) {
            EBCL_errPrint("Got unexpected response length from Crinit.");
            EBCL_destroyRtimCmd(&res);
            return -1;
        }

        char *endPtr = NULL;

        v->major = (uint8_t)strtoul(res.args[1], &endPtr, 10);
        if (endPtr == res.args[1]) {
            EBCL_errPrint("Could not convert major version number to integer.");
            EBCL_destroyRtimCmd(&res);
            return -1;
        }

        v->minor = (uint8_t)strtoul(res.args[2], &endPtr, 10);
        if (endPtr == res.args[2]) {
            EBCL_errPrint("Could not convert minor version number to integer.");
            EBCL_destroyRtimCmd(&res);
            return -1;
        }

        v->micro = (uint8_t)strtoul(res.args[3], &endPtr, 10);
        if (endPtr == res.args[3]) {
            EBCL_errPrint("Could not convert micro version number to integer.");
            EBCL_destroyRtimCmd(&res);
            return -1;
        }

        strncpy(v->git, res.args[4], sizeof(v->git) - 1);
        v->git[sizeof(v->git) - 1] = '\0';
    }

    EBCL_destroyRtimCmd(&res);
    return ret;
}

static inline int EBCL_crinitResponseCheck(const ebcl_RtimCmd_t *res, ebcl_RtimOp_t resCode) {
    if (res == NULL) {
        EBCL_errPrint("Pointer arguments must not be NULL.");
        return -1;
    }
    if (res->op != resCode) {
        EBCL_errPrint("Got unexpected response opcode from Crinit: %d", res->op);
        return -1;
    }

    if (res->argc == 0) {
        EBCL_errPrint("Got unexpected response length from Crinit.");
        return -1;
    }

    if (strcmp(res->args[0], EBCL_RTIMCMD_RES_OK) == 0) {
        return 0;
    }

    EBCL_errPrint("Crinit responded with an error message.");
    if (res->argc >= 2) {
        EBCL_errPrint("Message from Crinit: \'%s\'", res->args[1]);
    }

    return -1;
}
