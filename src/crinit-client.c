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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "crinit-client-internal.h"

/**
 * Attribute macro for exported/visible functions, used together with -fvisibility=hidden to export only selected
 * functions
 */
#define LIB_EXPORTED __attribute__((visibility("default")))
/** Attribute macro for a function executed on loading of the shared library. **/
#define LIB_CONSTRUCTOR __attribute__((constructor)) 
/** Attribute macro for a function executed on program exit if the shared library has been loaded before. **/
#define LIB_DESTRUCTOR __attribute__((destructor)) 
/** String to be used if no task name for sd_notify() is currently set. **/
#define EBCL_CRINIT_ENV_NOTIFY_NAME_UNDEF "@undefined"

/** Holds the task name for sd_notify() **/
static const char *notifyName = EBCL_CRINIT_ENV_NOTIFY_NAME_UNDEF;
/** Holds the path to the Crinit AF_UNIX socket file **/
static const char *crinitSockFile = EBCL_CRINIT_SOCKFILE;

/**
 * Perform a request/response transfer with Crinit
 *
 * Will connect to Crinit using EBCL_crinitConnect(), send a request/command using EBCL_crinitSend(), and receive the
 * result/response using EBCL_crinitRecv(). The server side equivalent is connThread() in notiserv.c.
 *
 * The following image shows the high level communication sequence. For the lower level, refer to
 * EBCL_crinitSend() and EBCL_crinitRecv().
 *
 * \image html notiserv_sock_comm_seq.svg
 *
 * @param res  Return pointer for response/result.
 * @param cmd  The command/request to send.
 *
 * @return 0 on success, -1 otherwise
 */
static inline int crinitXfer(ebcl_RtimCmd *res, const ebcl_RtimCmd *cmd);
/**
 * Check if a response from Crinit is valid and/or an error.
 *
 * @param res      The response to check.
 * @param resCode  The expected response opcode.
 *
 * @return 0 if \a res is valid and indicates success, -1 if not
 */
static inline int crinitResponseCheck(const ebcl_RtimCmd *res, ebcl_RtimOp resCode);

/**
 * Library initialization function.
 *
 * Gets called on shared object loading through #LIB_CONSTRUCTOR attribute. Initializes options to default values and
 * tries to get the task name for sd_notify() from the environment if it is present.
 */
static LIB_CONSTRUCTOR void libInit(void) {
    bool v = false;
    EBCL_setPrintPrefix("");
    EBCL_globOptSetBoolean(EBCL_GLOBOPT_DEBUG, &v);
    const char *envNotifyName = getenv(EBCL_CRINIT_ENV_NOTIFY_NAME);
    if (envNotifyName != NULL) {
        notifyName = envNotifyName;
    } else {
        notifyName = EBCL_CRINIT_ENV_NOTIFY_NAME_UNDEF;
    }
}

/**
 * Library cleanup function.
 *
 * Gets called on program end through #LIB_DESTRUCTOR attribute if the shared library was loaded. Frees global option
 * memory allocated as a consequence of libInit().
 */
static LIB_DESTRUCTOR void libDestroy(void) {
    EBCL_globOptDestroy();
}

LIB_EXPORTED int EBCL_crinitSetVerbose(bool v) {
    return EBCL_globOptSetBoolean(EBCL_GLOBOPT_DEBUG, &v);
}

LIB_EXPORTED void EBCL_crinitSetErrStream(FILE *errStream) {
    EBCL_setErrStream(errStream);
}

LIB_EXPORTED void EBCL_crinitSetInfoStream(FILE *infoStream) {
    EBCL_setInfoStream(infoStream);
}

LIB_EXPORTED void EBCL_crinitSetNotifyTaskName(const char *taskName) {
    if (taskName != NULL) {
        notifyName = taskName;
    }
}

LIB_EXPORTED void EBCL_crinitSetSocketPath(const char *sockFile) {
    if (sockFile != NULL) {
        crinitSockFile = sockFile;
    }
}

LIB_EXPORTED int sd_notify(int unset_environment, const char *state) {
    return sd_notifyf(unset_environment, state);
}

LIB_EXPORTED int sd_notifyf(int unset_environment, const char *format, ...) {
    if (format == NULL) {
        EBCL_errPrint("Format string must not be NULL");
        return -1;
    }

    if (unset_environment) {
        EBCL_errPrint("SD_NOTIFY: unset_environment is unimplemented.");
    }

    va_list vargs, vargsCopy;
    va_start(vargs, format);
    va_copy(vargsCopy, vargs);
    size_t n = vsnprintf(NULL, 0, format, vargs) + 1;
    va_end(vargs);
    char *argStr = malloc(n);
    if (argStr == NULL) {
        EBCL_errPrint("Could not allocate memory for SD_NOTIFY command string.");
        return -1;
    }
    vsnprintf(argStr, n, format, vargsCopy);
    va_end(vargsCopy);

    ebcl_RtimCmd cmd, res;
    if (EBCL_buildRtimCmd(&cmd, EBCL_RTIMCMD_C_NOTIFY, 2, notifyName, argStr) == -1) {
        free(argStr);
        EBCL_errPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }
    free(argStr);

    if (crinitXfer(&res, &cmd) == -1) {
        EBCL_destroyRtimCmd(&cmd);
        EBCL_errPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    EBCL_destroyRtimCmd(&cmd);

    int ret = crinitResponseCheck(&res, EBCL_RTIMCMD_R_NOTIFY);
    EBCL_destroyRtimCmd(&res);
    return ret;

    return 0;
}

LIB_EXPORTED int EBCL_crinitTaskAdd(const char *configFilePath, bool overwrite, const char *forceDeps) {
    if (configFilePath == NULL) {
        EBCL_errPrint("Config file path must not be NULL");
        return -1;
    }

    ebcl_RtimCmd cmd, res;
    const char *overwrStr = (overwrite) ? "true" : "false";
    if (forceDeps == NULL) {
        forceDeps = "@unchanged";
    }

    if (strcmp(forceDeps, "") == 0) {
        forceDeps = "@empty";
    }

    if (EBCL_buildRtimCmd(&cmd, EBCL_RTIMCMD_C_ADD, 3, configFilePath, overwrStr, forceDeps) == -1) {
        EBCL_errPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (crinitXfer(&res, &cmd) == -1) {
        EBCL_destroyRtimCmd(&cmd);
        EBCL_errPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    EBCL_destroyRtimCmd(&cmd);

    int ret = crinitResponseCheck(&res, EBCL_RTIMCMD_R_ADD);
    EBCL_destroyRtimCmd(&res);
    return ret;
}

LIB_EXPORTED int EBCL_crinitTaskEnable(const char *taskName) {
    if (taskName == NULL) {
        EBCL_errPrint("Task name must not be NULL");
        return -1;
    }

    ebcl_RtimCmd cmd, res;
    if (EBCL_buildRtimCmd(&cmd, EBCL_RTIMCMD_C_ENABLE, 1, taskName) == -1) {
        EBCL_errPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (crinitXfer(&res, &cmd) == -1) {
        EBCL_destroyRtimCmd(&cmd);
        EBCL_errPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    EBCL_destroyRtimCmd(&cmd);

    int ret = crinitResponseCheck(&res, EBCL_RTIMCMD_R_ENABLE);
    EBCL_destroyRtimCmd(&res);
    return ret;
}

LIB_EXPORTED int EBCL_crinitTaskDisable(const char *taskName) {
    if (taskName == NULL) {
        EBCL_errPrint("Task name must not be NULL");
        return -1;
    }
    ebcl_RtimCmd cmd, res;
    if (EBCL_buildRtimCmd(&cmd, EBCL_RTIMCMD_C_DISABLE, 1, taskName) == -1) {
        EBCL_errPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (crinitXfer(&res, &cmd) == -1) {
        EBCL_destroyRtimCmd(&cmd);
        EBCL_errPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    EBCL_destroyRtimCmd(&cmd);

    int ret = crinitResponseCheck(&res, EBCL_RTIMCMD_R_DISABLE);
    EBCL_destroyRtimCmd(&res);
    return ret;
}

LIB_EXPORTED int EBCL_crinitTaskStop(const char *taskName) {
    if (taskName == NULL) {
        EBCL_errPrint("Task name must not be NULL");
        return -1;
    }

    ebcl_RtimCmd cmd, res;
    if (EBCL_buildRtimCmd(&cmd, EBCL_RTIMCMD_C_STOP, 1, taskName) == -1) {
        EBCL_errPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (crinitXfer(&res, &cmd) == -1) {
        EBCL_destroyRtimCmd(&cmd);
        EBCL_errPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    EBCL_destroyRtimCmd(&cmd);

    int ret = crinitResponseCheck(&res, EBCL_RTIMCMD_R_STOP);
    EBCL_destroyRtimCmd(&res);
    return ret;
}

LIB_EXPORTED int EBCL_crinitTaskKill(const char *taskName) {
    if (taskName == NULL) {
        EBCL_errPrint("Task name must not be NULL");
        return -1;
    }

    ebcl_RtimCmd cmd, res;
    if (EBCL_buildRtimCmd(&cmd, EBCL_RTIMCMD_C_KILL, 1, taskName) == -1) {
        EBCL_errPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (crinitXfer(&res, &cmd) == -1) {
        EBCL_destroyRtimCmd(&cmd);
        EBCL_errPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    EBCL_destroyRtimCmd(&cmd);

    int ret = crinitResponseCheck(&res, EBCL_RTIMCMD_R_KILL);
    EBCL_destroyRtimCmd(&res);
    return ret;
}

LIB_EXPORTED int EBCL_crinitTaskRestart(const char *taskName) {
    if (taskName == NULL) {
        EBCL_errPrint("Task name must not be NULL");
        return -1;
    }

    ebcl_RtimCmd cmd, res;
    if (EBCL_buildRtimCmd(&cmd, EBCL_RTIMCMD_C_RESTART, 1, taskName) == -1) {
        EBCL_errPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (crinitXfer(&res, &cmd) == -1) {
        EBCL_destroyRtimCmd(&cmd);
        EBCL_errPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    EBCL_destroyRtimCmd(&cmd);

    int ret = crinitResponseCheck(&res, EBCL_RTIMCMD_R_RESTART);
    EBCL_destroyRtimCmd(&res);
    return ret;
}

LIB_EXPORTED int EBCL_crinitTaskGetStatus(ebcl_TaskState *s, pid_t *pid, const char *taskName) {
    if (taskName == NULL || s == NULL) {
        EBCL_errPrint("Pointer arguments must not be NULL");
        return -1;
    }

    ebcl_RtimCmd cmd, res;
    if (EBCL_buildRtimCmd(&cmd, EBCL_RTIMCMD_C_STATUS, 1, taskName) == -1) {
        EBCL_errPrint("Could not build RtimCmd to send to Crinit.");
        return -1;
    }

    if (crinitXfer(&res, &cmd) == -1) {
        EBCL_destroyRtimCmd(&cmd);
        EBCL_errPrint("Could not complete data transfer from/to Crinit.");
        return -1;
    }
    EBCL_destroyRtimCmd(&cmd);

    if (crinitResponseCheck(&res, EBCL_RTIMCMD_R_STATUS) == 0 && res.argc == 3) {
        *s = strtoul(res.args[1], NULL, 10);
        *pid = strtol(res.args[2], NULL, 10);
        EBCL_destroyRtimCmd(&res);
        return 0;
    }
    EBCL_destroyRtimCmd(&res);
    return -1;
}

static inline int crinitXfer(ebcl_RtimCmd *res, const ebcl_RtimCmd *cmd) {
    if (res == NULL || cmd == NULL) {
        EBCL_errPrint("Pointer arguments must not be NULL");
        return -1;
    }
    int sockFd = -1;
    if (EBCL_crinitConnect(&sockFd, crinitSockFile) == -1) {
        EBCL_errPrint("Could not connect to Crinit using socket at \'%s\'.", crinitSockFile);
        return -1;
    }
    EBCL_dbgInfoPrint("Connected to Crinit using %s.", crinitSockFile);
    if (EBCL_crinitSend(sockFd, cmd) == -1) {
        EBCL_errPrint("Could not send RtimCmd to Crinit.");
        return -1;
    }
    if (EBCL_crinitRecv(sockFd, res) == -1) {
        EBCL_errPrint("Could not receive response from Crinit.");
        return -1;
    }
    close(sockFd);
    return 0;
}

static inline int crinitResponseCheck(const ebcl_RtimCmd *res, ebcl_RtimOp resCode) {
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
