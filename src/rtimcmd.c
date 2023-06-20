/**
 * @file rtimcmd.c
 * @brief Implementation of runtime commands to be triggered by the notification/service interface.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "rtimcmd.h"

#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "common.h"
#include "fseries.h"
#include "globopt.h"
#include "logio.h"
#include "procdip.h"
#include "version.h"

/**
 * Argument structure for shdnThread().
 */
typedef struct ebcl_ShdnThrArgs_t {
    ebcl_TaskDB_t *ctx;  ///< TaskDB which holds the tasks to be terminated/killed on shutdown.
    int shutdownCmd;     ///< The command for the reboot() syscall, see documentation of RB_* macros in man 7 reboot.
} ebcl_ShdnThrArgs_t;

/**
 * A linked list to organize mount points that need to be handled before shutdown/reboot.
 */
typedef struct ebcl_UnMountList_t {
    struct ebcl_UnMountList_t *next;  ///< Pointer to next element.
    char target[PATH_MAX];            ///< A mount point path.
} ebcl_UnMountList_t;

/**
 * Internal implementation of the "addtask" command on an ebcl_TaskDB_t.
 *
 * For documentation on the command itself, see crinitClientTaskAdd().
 *
 * @param ctx  The ebcl_TaskDB_t to operate on.
 * @param res  Return pointer for response/result.
 * @param cmd  The ebcl_RtimCmd_t to execute, used to pass the argument list.
 *
 * @return 0 on success, -1 on error
 */
static int EBCL_execRtimCmdAddTask(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd);
/**
 * Internal implementation of the "addseries" command on an ebcl_TaskDB.
 *
 * For documentation on the command itself, see crinitClientSeriesAdd().
 *
 * @param ctx  The ebcl_TaskDB_t to operate on.
 * @param res  Return pointer for response/result.
 * @param cmd  The ebcl_RtimCmd_t to execute, used to pass the argument list.
 *
 * @return 0 on success, -1 on error
 */
static int EBCL_execRtimCmdAddSeries(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd);
/**
 * Internal implementation of the "enable" command on an ebcl_TaskDB_t.
 *
 * For documentation on the command itself, see crinitClientTaskEnable().
 *
 * @param ctx  The ebcl_TaskDB_t to operate on.
 * @param res  Return pointer for response/result.
 * @param cmd  The ebcl_RtimCmd_t to execute, used to pass the argument list.
 *
 * @return 0 on success, -1 on error
 */
static int EBCL_execRtimCmdEnable(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd);
/**
 * Internal implementation of the "disable" command on an ebcl_TaskDB_t.
 *
 * For documentation on the command itself, see crinitClientTaskDisable().
 *
 * @param ctx  The ebcl_TaskDB_t to operate on.
 * @param res  Return pointer for response/result.
 * @param cmd  The ebcl_RtimCmd_t to execute, used to pass the argument list.
 *
 * @return 0 on success, -1 on error
 */
static int EBCL_execRtimCmdDisable(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd);
/**
 * Internal implementation of the "stop" command on an ebcl_TaskDB_t.
 *
 * For documentation on the command itself, see crinitClientTaskStop().
 *
 * @param ctx  The ebcl_TaskDB_t to operate on.
 * @param res  Return pointer for response/result.
 * @param cmd  The ebcl_RtimCmd_t to execute, used to pass the argument list.
 *
 * @return 0 on success, -1 on error
 */
static int EBCL_execRtimCmdStop(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd);
/**
 * Internal implementation of the "kill" command on an ebcl_TaskDB_t.
 *
 * For documentation on the command itself, see crinitClientTaskKill().
 *
 * @param ctx  The ebcl_TaskDB_t to operate on.
 * @param res  Return pointer for response/result.
 * @param cmd  The ebcl_RtimCmd_t to execute, used to pass the argument list.
 *
 * @return 0 on success, -1 on error
 */
static int EBCL_execRtimCmdKill(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd);
/**
 * Internal implementation of the "restart" command on an ebcl_TaskDB_t.
 *
 * For documentation on the command itself, see crinitClientTaskRestart().
 *
 * @param ctx  The ebcl_TaskDB_t to operate on.
 * @param res  Return pointer for response/result.
 * @param cmd  The ebcl_RtimCmd_t to execute, used to pass the argument list.
 *
 * @return 0 on success, -1 on error
 */
static int EBCL_execRtimCmdRestart(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd);
/**
 * Internal implementation of the "notify" command on an ebcl_TaskDB_t.
 *
 * For documentation on the command itself, see sd_notify() and sd_notifyf().
 *
 * @param ctx  The ebcl_TaskDB_t to operate on.
 * @param res  Return pointer for response/result.
 * @param cmd  The ebcl_RtimCmd_t to execute, used to pass the argument list.
 *
 * @return 0 on success, -1 on error
 */
static int EBCL_execRtimCmdNotify(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd);
/**
 * Internal implementation of the "status" command on an ebcl_TaskDB_t.
 *
 * For documentation on the command itself, see crinitClientTaskGetStatus().
 *
 * @param ctx  The ebcl_TaskDB_t to operate on.
 * @param res  Return pointer for response/result.
 * @param cmd  The ebcl_RtimCmd_t to execute, used to pass the argument list.
 *
 * @return 0 on success, -1 on error
 */
static int EBCL_execRtimCmdStatus(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd);
/**
 * Internal implementation of the "list" command on an ebcl_TaskDB_t.
 *
 * For documentation on the command itself, see crinitClientGetTaskList().
 *
 * @param ctx  The ebcl_TaskDB_t to operate on.
 * @param res  Return pointer for response/result.
 * @param cmd  The ebcl_RtimCmd_t to execute, used to pass the argument list.
 *
 * @return 0 on success, -1 on error
 */
static int EBCL_execRtimCmdTaskList(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd);

/**
 * Internal implementation of the version query from the client library to crinit.
 *
 * For documentation on the command itself, see crinitClientGetVersion().
 *
 * @param ctx  The ebcl_TaskDB_t to operate on.
 * @param res  Return pointer for response/result.
 * @param cmd  The ebcl_RtimCmd_t to execute, used to pass the argument list.
 *
 * @return 0 on success, -1 on error
 */
static int EBCL_execRtimCmdGetVer(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd);

/**
 * Internal implementation of the "shutdown" command.
 *
 * For documentation on the command itself, see crinitClientShutdown().
 *
 * @param ctx  The currently running ebcl_TaskDB, needed to inhibit spawning of new processes during shutdown.
 * @param res  Return pointer for response/result.
 * @param cmd  The ebcl_RtimCmd to execute, used to pass the argument list.
 *
 * @return 0 on success, -1 on error
 */
static int EBCL_execRtimCmdShutdown(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd);
/**
 * Shutdown thread function.
 *
 * Run as a pthread, so that EBCL_execRtimCmdShutdown() may return before the actual shutdown syscall is issued and the
 * requesting process is not blocked.
 *
 * @param args  Argument pointer, see ShdnThrArgs.
 */
static void *EBCL_shdnThread(void *args);
/**
 * Wait function using nanosleep for the shutdown grace period.
 *
 * @param micros  Number of microseconds to wait.
 *
 * @return  0 on success, -1 on error
 */
static inline int EBCL_gracePeriod(unsigned long long micros);
/**
 * Prepares mounted filesystems for shutdown.
 *
 * Performs MNT_DETACH if the file systems are not the root mount. The root mount is remounted read-only if it was
 * writable before.
 *
 * @return  0 on success, -1 on error
 */
static inline int EBCL_fsPrepareShutdown(void);
/**
 * Generates a list of mount points that should be unmounted before shutdown/reboot.
 *
 * The resulting list will include all entries from `/proc/mounts` whose source is not `none` and which are not the root
 * node. The list will be ordered newest mount first. Memory for the list elements will be allocated and needs to be
 * freed using EBCL_freeUnMountList() when no longer in use.
 *
 * Additionally, EBCL_genUnMountList() checks if the root mount entry is read-only, setting \a rootfsIsRo to true in
 * that case.
 *
 * @param um          Return pointer for the UnMountList.
 * @param rootfsIsRo  Return pointer indicating if we are on a read-only rootfs.
 *
 * @return  0 on success, -1 on error
 */
static inline int EBCL_genUnMountList(ebcl_UnMountList_t **um, bool *rootfsIsRo);
/**
 * Frees memory allocated for an ebcl_UnMountList_t by EBCL_genUnMountList()
 *
 * @param um  The ebcl_UnMountList_t to free.
 */
static inline void EBCL_freeUnMountList(ebcl_UnMountList_t *um);

int EBCL_parseRtimCmd(ebcl_RtimCmd_t *out, const char *cmdStr) {
    if (out == NULL || cmdStr == NULL) {
        crinitErrPrint("Pointer parameters must not be NULL.");
        return -1;
    }
    if (EBCL_rtimOpGetByOpStr(&out->op, cmdStr) == -1) {
        crinitErrPrint("Could not parse runtime command. Unknown or invalid opcode string.");
        return -1;
    }

    size_t cmdStrLen = strlen(cmdStr);
    const char *argStart = cmdStr;
    while (*argStart != EBCL_RTIMCMD_ARGDELIM && *argStart != '\0') {
        argStart++;
    }
    // Ignore multiple EBCL_RTIMCMD_ARGDELIM in a row.
    while (*argStart != '\0' && *(argStart + 1) == EBCL_RTIMCMD_ARGDELIM) {
        argStart++;
    }
    const char *argEnd = &cmdStr[cmdStrLen];

    size_t argCount = 0;
    if (argStart < argEnd) {
        const char *runner = argStart;
        while (*runner != '\0') {
            if ((*runner == EBCL_RTIMCMD_ARGDELIM) && (*(runner + 1) != '\0') &&
                (*(runner + 1) != EBCL_RTIMCMD_ARGDELIM)) {
                argCount++;
            }
            runner++;
        }
    }
    out->args = malloc((argCount + 1) * sizeof(char *));
    if (out->args == NULL) {
        crinitErrnoPrint("Could not allocate memory for runtime commmand argument array with %zu arguments.", argCount);
        return -1;
    }
    if (argCount == 0) {
        out->args[0] = NULL;
        out->argc = 0;
        return 0;
    }

    size_t argStrLen = argEnd - argStart;
    out->args[0] = malloc(argStrLen);
    if (out->args[0] == NULL) {
        crinitErrnoPrint("Could not allocate memory for backing string of argument array");
        free(out->args);
        out->args = NULL;
        out->argc = 0;
        return -1;
    }
    memcpy(out->args[0], argStart + 1, argStrLen - 1);
    out->args[0][argStrLen - 1] = '\0';
    char *strtokState = NULL;
    char *token = NULL;
    char *start = out->args[0];
    size_t i = 0;
    char tokenList[2] = {EBCL_RTIMCMD_ARGDELIM, '\0'};
    while ((token = strtok_r(start, tokenList, &strtokState)) != NULL && i < argCount) {
        start = NULL;
        out->args[i] = token;
        i++;
    }
    out->args[argCount] = NULL;
    out->argc = argCount;
    return 0;
}

int EBCL_rtimCmdToMsgStr(char **out, size_t *outLen, const ebcl_RtimCmd_t *cmd) {
    if (out == NULL || outLen == NULL || cmd == NULL) {
        crinitErrPrint("Pointer parameters must not be NULL.");
        return -1;
    }

    const char *opStr = NULL;
    if (EBCL_opStrGetByRtimOp(&opStr, cmd->op) == -1) {
        crinitErrPrint("Could not get a string representation of the command's opcode.");
        return -1;
    }
    *outLen = strlen(opStr) + 1;
    for (size_t i = 0; i < cmd->argc; i++) {
        *outLen += strlen(cmd->args[i]) + 1;
    }

    *out = malloc(*outLen);
    if (*out == NULL) {
        crinitErrPrint("Could not allocate memory (%zu Bytes) for string representation of runtime command.", *outLen);
        *outLen = 0;
        return -1;
    }
    char *runner = *out;
    runner = stpcpy(runner, opStr);
    for (size_t i = 0; i < cmd->argc; i++) {
        *runner = EBCL_RTIMCMD_ARGDELIM;
        runner++;
        runner = stpcpy(runner, cmd->args[i]);
    }
    return 0;
}

int EBCL_execRtimCmd(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd) {
    if (res == NULL || cmd == NULL) {
        crinitErrPrint("Pointer parameters must not be NULL.");
        return -1;
    }
    switch (cmd->op) {
        case EBCL_RTIMCMD_C_ADDTASK:
            if (EBCL_execRtimCmdAddTask(ctx, res, cmd) == -1) {
                crinitErrPrint("Could not execute runtime command \'ADDTASK\'.");
                return -1;
            }
            return 0;
        case EBCL_RTIMCMD_C_ADDSERIES:
            if (EBCL_execRtimCmdAddSeries(ctx, res, cmd) == -1) {
                crinitErrPrint("Could not execute runtime command \'ADDSERIES\'.");
                return -1;
            }
            return 0;
        case EBCL_RTIMCMD_C_ENABLE:
            if (EBCL_execRtimCmdEnable(ctx, res, cmd) == -1) {
                crinitErrPrint("Could not execute runtime command \'ENABLE\'.");
                return -1;
            }
            return 0;
        case EBCL_RTIMCMD_C_DISABLE:
            if (EBCL_execRtimCmdDisable(ctx, res, cmd) == -1) {
                crinitErrPrint("Could not execute runtime command \'DISABLE\'.");
                return -1;
            }
            return 0;
        case EBCL_RTIMCMD_C_STOP:
            if (EBCL_execRtimCmdStop(ctx, res, cmd) == -1) {
                crinitErrPrint("Could not execute runtime command \'STOP\'.");
                return -1;
            }
            return 0;
        case EBCL_RTIMCMD_C_KILL:
            if (EBCL_execRtimCmdKill(ctx, res, cmd) == -1) {
                crinitErrPrint("Could not execute runtime command \'KILL\'.");
                return -1;
            }
            return 0;
        case EBCL_RTIMCMD_C_RESTART:
            if (EBCL_execRtimCmdRestart(ctx, res, cmd) == -1) {
                crinitErrPrint("Could not execute runtime command \'RESTART\'.");
                return -1;
            }
            return 0;
        case EBCL_RTIMCMD_C_NOTIFY:
            if (EBCL_execRtimCmdNotify(ctx, res, cmd) == -1) {
                crinitErrPrint("Could not execute runtime command \'NOTIFY\'.");
                return -1;
            }
            return 0;
        case EBCL_RTIMCMD_C_STATUS:
            if (EBCL_execRtimCmdStatus(ctx, res, cmd) == -1) {
                crinitErrPrint("Could not execute runtime command \'STATUS\'.");
                return -1;
            }
            return 0;
        case EBCL_RTIMCMD_C_TASKLIST:
            if (EBCL_execRtimCmdTaskList(ctx, res, cmd) == -1) {
                crinitErrPrint("Could not execute runtime command \'TASKLIST\'.");
                return -1;
            }
            return 0;
        case EBCL_RTIMCMD_C_SHUTDOWN:
            if (EBCL_execRtimCmdShutdown(ctx, res, cmd) == -1) {
                crinitErrPrint("Could not execute runtime command \'SHUTDOWN\'.");
                return -1;
            }
            return 0;
        case EBCL_RTIMCMD_C_GETVER:
            if (EBCL_execRtimCmdGetVer(ctx, res, cmd) == -1) {
                crinitErrPrint("Could not execute runtime command \'GETVER\'.");
                return -1;
            }
            return 0;

        case EBCL_RTIMCMD_R_ADDTASK:
        case EBCL_RTIMCMD_R_ADDSERIES:
        case EBCL_RTIMCMD_R_ENABLE:
        case EBCL_RTIMCMD_R_DISABLE:
        case EBCL_RTIMCMD_R_STOP:
        case EBCL_RTIMCMD_R_KILL:
        case EBCL_RTIMCMD_R_RESTART:
        case EBCL_RTIMCMD_R_NOTIFY:
        case EBCL_RTIMCMD_R_STATUS:
        case EBCL_RTIMCMD_R_TASKLIST:
        case EBCL_RTIMCMD_R_SHUTDOWN:
        case EBCL_RTIMCMD_R_GETVER:
        default:
            crinitErrPrint("Could not execute opcode %d. This is an unknown opcode or a response code.", cmd->op);
            return -1;
    }
    return 0;
}

int EBCL_buildRtimCmd(ebcl_RtimCmd_t *c, ebcl_RtimOp_t op, size_t argc, ...) {
    if (c == NULL) {
        crinitErrPrint("Return pointer must not be NULL.");
        return -1;
    }

    c->args = malloc((argc + 1) * sizeof(char *));
    if (c->args == NULL) {
        crinitErrPrint("Could not allocate memory for RtimCmd argument array.");
        return -1;
    }
    c->args[argc] = NULL;

    va_list vargs;
    va_start(vargs, argc);
    va_list vargsCopy;
    va_copy(vargsCopy, vargs);
    size_t sumStrSize = 0;
    for (size_t i = 0; i < argc; i++) {
        const char *str = va_arg(vargs, const char *);
        sumStrSize += strlen(str) + 1;
    }
    va_end(vargs);

    c->args[0] = malloc(sumStrSize);
    if (c->args[0] == NULL) {
        crinitErrPrint("Could not allocate memory for RtimCmd argument array backing string.");
        free(c->args);
        c->args = NULL;
        va_end(vargsCopy);
        return -1;
    }

    char *runner = c->args[0];
    for (size_t i = 0; i < argc; i++) {
        const char *str = va_arg(vargsCopy, const char *);
        size_t copyLen = strlen(str) + 1;
        memcpy(runner, str, copyLen);
        c->args[i] = runner;
        runner += copyLen;
    }
    va_end(vargsCopy);

    c->op = op;
    c->argc = argc;

    return 0;
}

int EBCL_buildRtimCmdArray(ebcl_RtimCmd_t *c, ebcl_RtimOp_t op, int argc, const char *args[]) {
    if (c == NULL) {
        crinitErrPrint("Return pointer must not be NULL.");
        return -1;
    }

    c->args = malloc((argc + 1) * sizeof(char *));
    if (c->args == NULL) {
        crinitErrPrint("Could not allocate memory for RtimCmd argument array.");
        return -1;
    }
    c->args[argc] = NULL;

    size_t sumStrSize = 0;
    for (int i = 0; i < argc; i++) {
        const char *str = args[i];
        sumStrSize += strlen(str) + 1;
    }

    c->args[0] = malloc(sumStrSize);
    if (c->args[0] == NULL) {
        crinitErrPrint("Could not allocate memory for RtimCmd argument array backing string.");
        free(c->args);
        c->args = NULL;
        return -1;
    }

    char *runner = c->args[0];
    for (int i = 0; i < argc; i++) {
        const char *str = args[i];
        size_t copyLen = strlen(str) + 1;
        memcpy(runner, str, copyLen);
        c->args[i] = runner;
        runner += copyLen;
    }

    c->op = op;
    c->argc = argc;

    return 0;
}

int EBCL_destroyRtimCmd(ebcl_RtimCmd_t *c) {
    if (c == NULL) {
        crinitErrPrint("RtimCmd pointer must not be NULL.");
        return -1;
    }
    free(c->args[0]);
    free(c->args);
    return 0;
}

static int EBCL_execRtimCmdAddTask(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd) {
    if (ctx == NULL || res == NULL || cmd == NULL) {
        crinitErrPrint("Pointer parameters must not be NULL.");
    }

    crinitDbgInfoPrint("Will execute runtime command \'ADDTASK\' with following arguments:");
    for (size_t i = 0; i < cmd->argc; i++) {
        crinitDbgInfoPrint("    args[%zu] = %s", i, cmd->args[i]);
    }

    if (cmd->argc != 3) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_ADDTASK, 2, EBCL_RTIMCMD_RES_ERR, "Wrong number of arguments.");
    }

    ebcl_ConfKvList_t *c;
    if (EBCL_parseConf(&c, cmd->args[0]) == -1) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_ADDTASK, 2, EBCL_RTIMCMD_RES_ERR, "Could not parse given config.");
    }
    crinitDbgInfoPrint("File \'%s\' loaded.", cmd->args[0]);

    if (strcmp(cmd->args[2], "@unchanged") != 0) {
        if (strcmp(cmd->args[2], "@empty") == 0) {
            if (EBCL_confListSetVal("", "DEPENDS", c) == -1) {
                return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_ADDTASK, 2, EBCL_RTIMCMD_RES_ERR,
                                         "Could not set dependencies to empty.");
            }
        } else {
            if (EBCL_confListSetVal(cmd->args[2], "DEPENDS", c) == -1) {
                EBCL_freeConfList(c);
                return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_ADDTASK, 2, EBCL_RTIMCMD_RES_ERR,
                                         "Could not set dependencies to given string.");
            }
        }
    }

    crinitTask_t *t = NULL;
    if (crinitTaskCreateFromConfKvList(&t, c) == -1) {
        EBCL_freeConfList(c);
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_ADDTASK, 2, EBCL_RTIMCMD_RES_ERR,
                                 "Could not create task from config.");
    }
    EBCL_freeConfList(c);

    crinitDbgInfoPrint("Task extracted without error.");
    bool overwrite = false;
    if (strcmp(cmd->args[1], "true") == 0) {
        overwrite = true;
    }

    if (EBCL_taskDBInsert(ctx, t, overwrite) == -1) {
        crinitFreeTask(t);
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_ADDTASK, 2, EBCL_RTIMCMD_RES_ERR,
                                 "Could not insert new task into TaskDB.");
    }
    crinitFreeTask(t);
    return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_ADDTASK, 1, EBCL_RTIMCMD_RES_OK);
}

static int EBCL_execRtimCmdAddSeries(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd) {
    if (ctx == NULL || res == NULL || cmd == NULL) {
        crinitErrPrint("Pointer parameters must not be NULL.");
    }

    crinitDbgInfoPrint("Will execute runtime command \'ADDSERIES\' with following arguments:");
    for (size_t i = 0; i < cmd->argc; i++) {
        crinitDbgInfoPrint("    args[%zu] = %s", i, cmd->args[i]);
    }

    if (cmd->argc != 2) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_ADDSERIES, 2, EBCL_RTIMCMD_RES_ERR, "Wrong number of arguments.");
    }

    if (!crinitIsAbsPath(cmd->args[0])) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_ADDSERIES, 2, EBCL_RTIMCMD_RES_ERR,
                                 "Path to series file must be absolute.");
    }

    if (EBCL_taskDBSetSpawnInhibit(ctx, true) == -1) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_ADDSERIES, 2, EBCL_RTIMCMD_RES_ERR,
                                 "Could not inhibit process spawning to load new series file.");
    }

    crinitFileSeries_t taskSeries;
    if (EBCL_loadSeriesConf(&taskSeries, cmd->args[0]) == -1) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_ADDSERIES, 2, EBCL_RTIMCMD_RES_ERR, "Could not load series file.");
    }

    bool overwriteTasks = false;
    if (strcmp(cmd->args[1], "true") == 0) {
        overwriteTasks = true;
    }

    for (size_t n = 0; n < taskSeries.size; n++) {
        char *confFn = taskSeries.fnames[n];
        bool confFnAllocated = false;
        if (!crinitIsAbsPath(confFn)) {
            size_t prefixLen = strlen(taskSeries.baseDir);
            size_t suffixLen = strlen(taskSeries.fnames[n]);
            confFn = malloc(prefixLen + suffixLen + 2);
            if (confFn == NULL) {
                crinitDestroyFileSeries(&taskSeries);
                EBCL_taskDBSetSpawnInhibit(ctx, false);
                return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_ADDSERIES, 2, EBCL_RTIMCMD_RES_ERR,
                                         "Memory allocation error.");
            }
            memcpy(confFn, taskSeries.baseDir, prefixLen);
            confFn[prefixLen] = '/';
            memcpy(confFn + prefixLen + 1, taskSeries.fnames[n], suffixLen + 1);
            confFnAllocated = true;
        }
        ebcl_ConfKvList_t *c;

        if (EBCL_parseConf(&c, confFn) == -1) {
            crinitErrPrint("Could not parse file \'%s\'.", confFn);
            if (confFnAllocated) {
                free(confFn);
            }
            crinitDestroyFileSeries(&taskSeries);
            EBCL_taskDBSetSpawnInhibit(ctx, false);
            return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_ADDSERIES, 2, EBCL_RTIMCMD_RES_ERR,
                                     "Could not parse config file.");
        }
        crinitInfoPrint("File \'%s\' loaded.", confFn);
        if (confFnAllocated) {
            free(confFn);
        }

        crinitTask_t *t = NULL;
        if (crinitTaskCreateFromConfKvList(&t, c) == -1) {
            EBCL_freeConfList(c);
            crinitDestroyFileSeries(&taskSeries);
            EBCL_taskDBSetSpawnInhibit(ctx, false);
            return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_ADDSERIES, 2, EBCL_RTIMCMD_RES_ERR,
                                     "Could not create task from config file.");
        }

        EBCL_freeConfList(c);
        if (EBCL_taskDBInsert(ctx, t, overwriteTasks) == -1) {
            crinitFreeTask(t);
            crinitDestroyFileSeries(&taskSeries);
            return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_ADDTASK, 2, EBCL_RTIMCMD_RES_ERR,
                                     "Could not insert new task into TaskDB.");
        }

        crinitFreeTask(t);
    }

    crinitDestroyFileSeries(&taskSeries);
    if (EBCL_taskDBSetSpawnInhibit(ctx, false) == -1) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_ADDSERIES, 2, EBCL_RTIMCMD_RES_ERR,
                                 "Could not re-enable spawning of processes.");
    }
    return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_ADDSERIES, 1, EBCL_RTIMCMD_RES_OK);
}

static int EBCL_execRtimCmdEnable(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd) {
    crinitDbgInfoPrint("Will execute runtime command \'ENABLE\' with following arguments:");
    for (size_t i = 0; i < cmd->argc; i++) {
        crinitDbgInfoPrint("    args[%zu] = %s", i, cmd->args[i]);
    }
    if (cmd->argc != 1) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_ENABLE, 2, EBCL_RTIMCMD_RES_ERR, "Wrong number of arguments.");
    }
    const char depStr[] = "@ctl\0enable";
    crinitTaskDep_t tempDep = {NULL, NULL};
    tempDep.name = malloc(sizeof(depStr));
    if (tempDep.name == NULL) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_ENABLE, 2, EBCL_RTIMCMD_RES_ERR, "Memory allocation error.");
    }
    memcpy(tempDep.name, depStr, sizeof(depStr));
    tempDep.event = tempDep.name + strlen(tempDep.name) + 1;

    if (EBCL_taskDBRemoveDepFromTask(ctx, &tempDep, cmd->args[0]) == -1) {
        free(tempDep.name);
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_ENABLE, 2, EBCL_RTIMCMD_RES_ERR,
                                 "Could not remove \'enable\' dependency from task.");
    }
    free(tempDep.name);
    return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_ENABLE, 1, EBCL_RTIMCMD_RES_OK);
}

static int EBCL_execRtimCmdDisable(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd) {
    crinitDbgInfoPrint("Will execute runtime command \'DISABLE\' with following arguments:");
    for (size_t i = 0; i < cmd->argc; i++) {
        crinitDbgInfoPrint("    args[%zu] = %s", i, cmd->args[i]);
    }
    if (cmd->argc != 1) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_DISABLE, 2, EBCL_RTIMCMD_RES_ERR, "Wrong number of arguments.");
    }
    const char newDepStr[] = "@ctl\0enable";
    crinitTaskDep_t tempDep = {NULL, NULL};
    tempDep.name = malloc(sizeof(newDepStr));
    if (tempDep.name == NULL) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_DISABLE, 2, EBCL_RTIMCMD_RES_ERR, "Memory allocation error.");
    }
    memcpy(tempDep.name, newDepStr, sizeof(newDepStr));
    tempDep.event = tempDep.name + strlen(tempDep.name) + 1;

    if (EBCL_taskDBAddDepToTask(ctx, &tempDep, cmd->args[0]) == -1) {
        free(tempDep.name);
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_DISABLE, 2, EBCL_RTIMCMD_RES_ERR,
                                 "Could not add dependency to task.");
    }
    free(tempDep.name);
    return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_DISABLE, 1, EBCL_RTIMCMD_RES_OK);
}

static int EBCL_execRtimCmdStop(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd) {
    crinitDbgInfoPrint("Will execute runtime command \'STOP\' with following arguments:");
    for (size_t i = 0; i < cmd->argc; i++) {
        crinitDbgInfoPrint("    args[%zu] = %s", i, cmd->args[i]);
    }
    if (cmd->argc != 1) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_STOP, 2, EBCL_RTIMCMD_RES_ERR, "Wrong number of arguments.");
    }
    if (EBCL_setInhibitWait(true) == -1) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_STOP, 2, EBCL_RTIMCMD_RES_ERR,
                                 "Could not inhibit waiting for processes.");
    }
    pid_t taskPid = 0;
    if (EBCL_taskDBGetTaskPID(ctx, &taskPid, cmd->args[0]) == -1) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_STOP, 2, EBCL_RTIMCMD_RES_ERR, "Could not access task.");
    }
    if (taskPid <= 0) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_STOP, 2, EBCL_RTIMCMD_RES_ERR, "No PID registered for task.");
    }
    if (kill(taskPid, SIGTERM) == -1) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_STOP, 2, EBCL_RTIMCMD_RES_ERR,
                                 "Could not send SIGTERM to process.");
    }
    if (EBCL_setInhibitWait(false) == -1) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_STOP, 2, EBCL_RTIMCMD_RES_ERR,
                                 "Could not reactivate waiting for processes.");
    }
    return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_STOP, 1, EBCL_RTIMCMD_RES_OK);
}

static int EBCL_execRtimCmdKill(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd) {
    crinitDbgInfoPrint("Will execute runtime command \'KILL\' with following arguments:");
    for (size_t i = 0; i < cmd->argc; i++) {
        crinitDbgInfoPrint("    args[%zu] = %s", i, cmd->args[i]);
    }
    if (cmd->argc != 1) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_KILL, 2, EBCL_RTIMCMD_RES_ERR, "Wrong number of arguments.");
    }

    if (EBCL_setInhibitWait(true) == -1) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_STOP, 2, EBCL_RTIMCMD_RES_ERR,
                                 "Could not inhibit waiting for processes.");
    }
    pid_t taskPid = 0;
    if (EBCL_taskDBGetTaskPID(ctx, &taskPid, cmd->args[0]) == -1) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_KILL, 2, EBCL_RTIMCMD_RES_ERR, "Could not access task.");
    }
    if (taskPid <= 0) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_KILL, 2, EBCL_RTIMCMD_RES_ERR, "No PID registered for task.");
    }
    if (kill(taskPid, SIGKILL) == -1) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_KILL, 2, EBCL_RTIMCMD_RES_ERR,
                                 "Could not send SIGKILL to Process.");
    }
    if (EBCL_setInhibitWait(false) == -1) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_STOP, 2, EBCL_RTIMCMD_RES_ERR,
                                 "Could not reactivate waiting for processes.");
    }
    return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_KILL, 1, EBCL_RTIMCMD_RES_OK);
}

static int EBCL_execRtimCmdRestart(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd) {
    crinitDbgInfoPrint("Will execute runtime command \'RESTART\' with following arguments:");
    for (size_t i = 0; i < cmd->argc; i++) {
        crinitDbgInfoPrint("    args[%zu] = %s", i, cmd->args[i]);
    }
    if (cmd->argc != 1) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_RESTART, 2, EBCL_RTIMCMD_RES_ERR, "Wrong number of arguments.");
    }

    ebcl_TaskState_t s = 0;
    if (EBCL_taskDBGetTaskState(ctx, &s, cmd->args[0]) == -1) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_RESTART, 2, EBCL_RTIMCMD_RES_ERR,
                                 "Could not get task state from TaskDB.");
    }

    if (!(s & (EBCL_TASK_STATE_DONE | EBCL_TASK_STATE_FAILED))) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_RESTART, 2, EBCL_RTIMCMD_RES_ERR,
                                 "Task is not either DONE or FAILED.");
    }

    if (EBCL_taskDBSetTaskState(ctx, 0, cmd->args[0]) == -1) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_RESTART, 2, EBCL_RTIMCMD_RES_ERR, "Could not reset task state.");
    }
    return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_RESTART, 1, EBCL_RTIMCMD_RES_OK);
}

static int EBCL_execRtimCmdNotify(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd) {
    crinitDbgInfoPrint("Will execute runtime command \'NOTIFY\' with following arguments:");
    for (size_t i = 0; i < cmd->argc; i++) {
        crinitDbgInfoPrint("    args[%zu] = %s", i, cmd->args[i]);
    }

    // Notify library will need to send task name. So process dispatch will need to set it in an environment for
    // execve.
    if (cmd->argc < 2) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_NOTIFY, 2, EBCL_RTIMCMD_RES_ERR, "Wrong number of arguments.");
    }

    pid_t pid = -1;
    int ready = -1;
    int stopping = -1;

    for (size_t i = 1; i < cmd->argc; i++) {
        size_t argLen = strlen(cmd->args[i]);
        const char *delim = strchr(cmd->args[i], '=');
        if (delim != NULL) {
            size_t cmpLen = delim - cmd->args[i];
            delim++;
            if (cmpLen < argLen) {
                if (strncmp(cmd->args[i], "MAINPID", cmpLen) == 0) {
                    pid = strtol(delim, NULL, 10);
                }
                if (strncmp(cmd->args[i], "READY", cmpLen) == 0) {
                    ready = strtol(delim, NULL, 10);
                }
                if (strncmp(cmd->args[i], "STOPPING", cmpLen) == 0) {
                    stopping = strtol(delim, NULL, 10);
                }
            }
        }
    }

    if (pid > 0) {
        if (EBCL_taskDBSetTaskPID(ctx, pid, cmd->args[0]) == -1) {
            return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_NOTIFY, 2, EBCL_RTIMCMD_RES_ERR,
                                     "Could not set main PID of task.");
        }
    }

    if (ready > 0) {
        const ebcl_TaskState_t s = EBCL_TASK_STATE_RUNNING | EBCL_TASK_STATE_NOTIFIED;
        char depEvent[] = CRINIT_TASK_EVENT_RUNNING CRINIT_TASK_EVENT_NOTIFY_SUFFIX;
        if (EBCL_taskDBSetTaskState(ctx, s, cmd->args[0]) == -1) {
            return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_NOTIFY, 2, EBCL_RTIMCMD_RES_ERR,
                                     "Could not set task state to RUNNING.");
        }

        crinitTaskDep_t dep = {cmd->args[0], depEvent};
        if (EBCL_taskDBFulfillDep(ctx, &dep) == -1) {
            return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_NOTIFY, 2, EBCL_RTIMCMD_RES_ERR,
                                     "Could not fulfill dependency \'%s:%s\'.", cmd->args[0], depEvent);
        }

        if (EBCL_taskDBProvideFeatureByTaskName(ctx, cmd->args[0], s) == -1) {
            return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_NOTIFY, 2, EBCL_RTIMCMD_RES_ERR,
                                     "Could not set features of spawned task \'%s\' as provided.", cmd->args[0]);
        }
    }

    if (stopping > 0) {
        const ebcl_TaskState_t s = EBCL_TASK_STATE_DONE | EBCL_TASK_STATE_NOTIFIED;
        char depEvent[] = CRINIT_TASK_EVENT_DONE CRINIT_TASK_EVENT_NOTIFY_SUFFIX;

        if (EBCL_taskDBSetTaskState(ctx, s, cmd->args[0]) == -1) {
            return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_NOTIFY, 2, EBCL_RTIMCMD_RES_ERR,
                                     "Could not set task state to DONE.");
        }

        const crinitTaskDep_t dep = {cmd->args[0], depEvent};
        if (EBCL_taskDBFulfillDep(ctx, &dep) == -1) {
            return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_NOTIFY, 2, EBCL_RTIMCMD_RES_ERR,
                                     "Could not fulfill dependency \'%s:%s\'.", cmd->args[0], depEvent);
        }

        if (EBCL_taskDBProvideFeatureByTaskName(ctx, cmd->args[0], s) == -1) {
            return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_NOTIFY, 2, EBCL_RTIMCMD_RES_ERR,
                                     "Could not set features of spawned task \'%s\' as provided.", cmd->args[0]);
        }
    }

    return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_NOTIFY, 1, EBCL_RTIMCMD_RES_OK);
}

static int EBCL_execRtimCmdStatus(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd) {
    crinitDbgInfoPrint("Will execute runtime command \'STATUS\' with following arguments:");
    for (size_t i = 0; i < cmd->argc; i++) {
        crinitDbgInfoPrint("    args[%zu] = %s", i, cmd->args[i]);
    }
    if (cmd->argc != 1) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_STATUS, 2, EBCL_RTIMCMD_RES_ERR, "Wrong number of arguments.");
    }
    ebcl_TaskState_t s = 0;
    pid_t pid = -1;

    if (EBCL_taskDBGetTaskStateAndPID(ctx, &s, &pid, cmd->args[0]) == -1) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_STATUS, 2, EBCL_RTIMCMD_RES_ERR,
                                 "Could not get state and PID of task.");
    }

    size_t resStrLen = snprintf(NULL, 0, "%lu\n%d", s, pid) + 1;
    char *resStr = malloc(resStrLen);
    if (resStr == NULL) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_STATUS, 2, EBCL_RTIMCMD_RES_ERR, "Memory allocation error.");
    }
    snprintf(resStr, resStrLen, "%lu\n%d", s, pid);
    char *pidStr = strchr(resStr, '\n');
    *pidStr = '\0';
    pidStr++;
    if (EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_STATUS, 3, EBCL_RTIMCMD_RES_OK, resStr, pidStr) == -1) {
        free(resStr);
        return -1;
    }
    free(resStr);
    return 0;
}

static int EBCL_execRtimCmdTaskList(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd) {
    crinitDbgInfoPrint("Will execute runtime command \'TASKLIST\' with following arguments:");
    for (size_t i = 0; i < cmd->argc; i++) {
        crinitDbgInfoPrint("    args[%zu] = %s", i, cmd->args[i]);
    }
    if (cmd->argc != 0) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_TASKLIST, 2, EBCL_RTIMCMD_RES_ERR, "Wrong number of arguments.");
    }

    int ret = 0;
    size_t numTasks;
    char **tasks;

    if (EBCL_taskDBExportTaskNamesToArray(ctx, &tasks, &numTasks) == -1) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_STATUS, 2, EBCL_RTIMCMD_RES_ERR, "Memory allocation error.");
    }

    const char **args = malloc((numTasks + 1) * sizeof(*args));
    if (args == NULL) {
        ret = EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_STATUS, 2, EBCL_RTIMCMD_RES_ERR, "Memory allocation error.");
        goto out;
    }

    args[0] = EBCL_RTIMCMD_RES_OK;
    for (size_t i = 0; i < numTasks; i++) {
        args[i + 1] = tasks[i];
    }

    ret = EBCL_buildRtimCmdArray(res, EBCL_RTIMCMD_R_TASKLIST, numTasks + 1, args);

out:
    free(args);
    for (size_t i = 0; i < numTasks; i++) {
        free(tasks[i]);
    }
    free(tasks);

    return ret;
}

static int EBCL_execRtimCmdGetVer(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd) {
    if (ctx == NULL || res == NULL || cmd == NULL) {
        crinitErrPrint("Pointer parameters must not be NULL");
        return -1;
    }

    crinitDbgInfoPrint("Will execute runtime command \'GETVER\'.");
    if (cmd->argc != 0) {
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_GETVER, 2, EBCL_RTIMCMD_RES_ERR, "Wrong number of arguments.");
    }

    char major[8] = {'\0'}, minor[8] = {'\0'}, micro[8] = {'\0'};
    snprintf(major, sizeof(major) - 1, "%u", crinitVersion.major);
    snprintf(minor, sizeof(minor) - 1, "%u", crinitVersion.minor);
    snprintf(micro, sizeof(micro) - 1, "%u", crinitVersion.micro);

    return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_GETVER, 5, EBCL_RTIMCMD_RES_OK, major, minor, micro,
                             crinitVersion.git);
}

static int EBCL_execRtimCmdShutdown(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd) {
    if (ctx == NULL || res == NULL || cmd == NULL) {
        crinitErrPrint("Pointer parameters must not be NULL");
        return -1;
    }
    ebcl_ShdnThrArgs_t *thrArgs = malloc(sizeof(ebcl_ShdnThrArgs_t));
    if (thrArgs == NULL) {
        crinitErrnoPrint("Could not allocate memory for shutdown thread arguments.");
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_SHUTDOWN, 2, EBCL_RTIMCMD_RES_ERR, "Memory allocation error.");
    }

    if (cmd->argc != 1) {
        free(thrArgs);
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_SHUTDOWN, 2, EBCL_RTIMCMD_RES_ERR, "Wrong number of arguments.");
    }

    thrArgs->ctx = ctx;
    errno = 0;
    thrArgs->shutdownCmd = (int)strtol(cmd->args[0], NULL, 16);
    if (thrArgs->shutdownCmd != RB_AUTOBOOT && thrArgs->shutdownCmd != RB_POWER_OFF) {
        free(thrArgs);
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_SHUTDOWN, 2, EBCL_RTIMCMD_RES_ERR, "Invalid argument.");
    }

    pthread_t shdnThreadRef;
    pthread_attr_t thrAttrs;
    pthread_attr_init(&thrAttrs);
    pthread_attr_setstacksize(&thrAttrs, EBCL_RTIMCMD_SHDN_THREAD_STACK_SIZE);
    pthread_attr_setdetachstate(&thrAttrs, PTHREAD_CREATE_DETACHED);
    errno = pthread_create(&shdnThreadRef, &thrAttrs, EBCL_shdnThread, thrArgs);
    pthread_attr_destroy(&thrAttrs);
    if (errno != 0) {
        crinitErrnoPrint("Could not start shutdown thread");
        free(thrArgs);
        return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_SHUTDOWN, 2, EBCL_RTIMCMD_RES_ERR,
                                 "Could not start shutdown thread.");
    }

    return EBCL_buildRtimCmd(res, EBCL_RTIMCMD_R_SHUTDOWN, 1, EBCL_RTIMCMD_RES_OK);
}

static void *EBCL_shdnThread(void *args) {
    ebcl_ShdnThrArgs_t *a = (ebcl_ShdnThrArgs_t *)args;
    ebcl_TaskDB_t *ctx = a->ctx;
    int shutdownCmd = a->shutdownCmd;
    free(args);

    if (EBCL_taskDBSetSpawnInhibit(ctx, true) == -1) {
        crinitErrPrint("Could not inhibit spawning of new tasks. Continuing anyway.");
    }

    unsigned long long gpMicros = EBCL_CONFIG_DEFAULT_SHDGRACEP;
    if (EBCL_globOptGetUnsignedLL(EBCL_GLOBOPT_SHDGRACEP, &gpMicros) == -1) {
        gpMicros = EBCL_CONFIG_DEFAULT_SHDGRACEP;
        crinitErrPrint("Could not read global option for shutdown grace period, using default: %lluus.", gpMicros);
    }

    kill(-1, SIGCONT);
    kill(-1, SIGTERM);
    crinitDbgInfoPrint("Sending SIGTERM to all processes.");
    if (EBCL_gracePeriod(gpMicros) == -1) {
        crinitErrPrint("Could not wait out the shutdown grace period, continuing anyway.");
    }
    kill(-1, SIGKILL);
    crinitDbgInfoPrint("Sending SIGKILL to all processes.");
    if (EBCL_fsPrepareShutdown() == -1) {
        crinitErrPrint(
            "Could not un- or remount filesystems cleanly, continuing anyway. Some filesystems may be dirty on "
            "next "
            "boot.");
    }
    if (reboot(shutdownCmd) == -1) {
        crinitErrnoPrint("Reboot syscall failed.");
    }
    return NULL;
}

static inline int EBCL_gracePeriod(unsigned long long micros) {
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) == -1) {
        crinitErrnoPrint("Could not get current time from monotonic clock.");
        return -1;
    }
    t.tv_sec += (time_t)(micros / 1000000uLL);
    unsigned long long nsec = ((micros % 1000000uLL) * 1000uLL) + (unsigned long long)t.tv_nsec;
    t.tv_sec += (time_t)(nsec / 1000000000uLL);
    t.tv_nsec = (long)(nsec % 1000000000uLL);
    int ret = 0;
    do {
        ret = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
    } while (ret == -1 && errno == EINTR);
    if (ret == -1) {
        crinitErrnoPrint("Could not sleep for %lluus.", micros);
        return -1;
    }
    return 0;
}

static inline int EBCL_genUnMountList(ebcl_UnMountList_t **ml, bool *rootfsIsRo) {
    if (ml == NULL || rootfsIsRo == NULL) {
        crinitErrPrint("Input parameters must not be NULL.");
        return -1;
    }
    *rootfsIsRo = false;
    FILE *mountListStream = fopen("/proc/mounts", "r");
    if (mountListStream == NULL) {
        crinitErrnoPrint("Could not open \'/proc/mounts\' for reading.");
        return -1;
    }
    ebcl_UnMountList_t *pList = malloc(sizeof(ebcl_UnMountList_t));
    if (pList == NULL) {
        crinitErrnoPrint("Could not allocate memory for list of mount points to be unmounted.");
        fclose(mountListStream);
        return -1;
    }
    pList->next = NULL;
    while (fgets(pList->target, sizeof(pList->target), mountListStream) != NULL) {
        char *strtokState = NULL;
        pList->target[sizeof(pList->target) - 1] = '\0';
        // Filter out things like tmpfs, proc, sysfs mounted from 'none'.
        char *runner = strtok_r(pList->target, " ", &strtokState);
        if (runner == NULL || strcmp(runner, "none") == 0) {
            pList->target[0] = '\0';
            continue;
        }
        // Mountpoint
        runner = strtok_r(NULL, " ", &strtokState);
        if (runner == NULL) {
            pList->target[0] = '\0';
            continue;
        }

        // If it is the root mount...
        if (strcmp(runner, "/") == 0) {
            // jump over fstype...
            runner = strtok_r(NULL, " ", &strtokState);
            if (runner == NULL) {
                pList->target[0] = '\0';
                continue;
            }
            // and check if it's already mounted read-only.
            runner = strtok_r(NULL, " ", &strtokState);
            if (runner == NULL || (strncmp(runner, "ro,", 3) != 0 && strncmp(runner, "ro ", 3) != 0)) {
                pList->target[0] = '\0';
                continue;
            }
            *rootfsIsRo = true;
        } else {
            // Put the mount point into the list in reverse order so we're beginning with the newest entry.
            memmove(pList->target, runner, strlen(runner) + 1);
            ebcl_UnMountList_t *new = malloc(sizeof(ebcl_UnMountList_t));
            if (new == NULL) {
                crinitErrnoPrint("Could not allocate memory for list of mount points to be unmounted.");
                fclose(mountListStream);
                EBCL_freeUnMountList(pList);
                return -1;
            }
            new->next = pList;
            new->target[0] = '\0';
            pList = new;
        }
    }
    *ml = pList;
    fclose(mountListStream);
    return 0;
}

static inline void EBCL_freeUnMountList(ebcl_UnMountList_t *ml) {
    ebcl_UnMountList_t *prev;
    while (ml != NULL) {
        prev = ml;
        ml = ml->next;
        free(prev);
    }
}

static inline int EBCL_fsPrepareShutdown(void) {
    int out = 0;
    ebcl_UnMountList_t *um = NULL;
    bool rootfsIsRo;

    if (EBCL_genUnMountList(&um, &rootfsIsRo) == -1) {
        crinitErrPrint(
            "Could not generate list of targets to unmount. Will at least try to remount root filesystem as "
            "read-only.");
        rootfsIsRo = false;
        out = -1;
    } else {
        ebcl_UnMountList_t *runner = um;
        while (runner != NULL) {
            if (runner->target[0] != '\0') {
                crinitDbgInfoPrint("Will unmount target \'%s\'.", runner->target);
                // Perform a lazy unmount of all targets in the list (does not include root node).
                if (umount2(runner->target, MNT_DETACH) == -1) {
                    crinitErrnoPrint("Could not umount (detach) mountpoint \'%s\'. Continuing anyway.", runner->target);
                    out = -1;
                }
            }
            runner = runner->next;
        }
        EBCL_freeUnMountList(um);
    }
    // If it is (possibly) an rw rootfs, try remounting it ro.
    if (!rootfsIsRo && mount(NULL, "/", NULL, MS_REMOUNT | MS_RDONLY, NULL) == -1) {
        crinitErrnoPrint("Could not remount rootfs read-only, continuing anyway. Filesystem may be dirty on boot.");
        out = -1;
    }
    sync();
    return out;
}
