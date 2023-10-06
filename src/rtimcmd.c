// SPDX-License-Identifier: MIT
/**
 * @file rtimcmd.c
 * @brief Implementation of runtime commands to be triggered by the notification/service interface.
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
typedef struct crinitShdnThrArgs_t {
    crinitTaskDB_t *ctx;  ///< TaskDB which holds the tasks to be terminated/killed on shutdown.
    int shutdownCmd;      ///< The command for the reboot() syscall, see documentation of RB_* macros in man 7 reboot.
} crinitShdnThrArgs_t;

/**
 * A linked list to organize mount points that need to be handled before shutdown/reboot.
 */
typedef struct crinitUnMountList_t {
    struct crinitUnMountList_t *next;  ///< Pointer to next element.
    char target[PATH_MAX];             ///< A mount point path.
} crinitUnMountList_t;

/**
 * Internal implementation of the "addtask" command on an crinitTaskDB_t.
 *
 * For documentation on the command itself, see crinitClientTaskAdd().
 *
 * @param ctx  The crinitTaskDB_t to operate on.
 * @param res  Return pointer for response/result.
 * @param cmd  The crinitRtimCmd_t to execute, used to pass the argument list.
 *
 * @return 0 on success, -1 on error
 */
static int crinitExecRtimCmdAddTask(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd);
/**
 * Internal implementation of the "addseries" command on an crinitTaskDB.
 *
 * For documentation on the command itself, see crinitClientSeriesAdd().
 *
 * @param ctx  The crinitTaskDB_t to operate on.
 * @param res  Return pointer for response/result.
 * @param cmd  The crinitRtimCmd_t to execute, used to pass the argument list.
 *
 * @return 0 on success, -1 on error
 */
static int crinitExecRtimCmdAddSeries(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd);
/**
 * Internal implementation of the "enable" command on an crinitTaskDB_t.
 *
 * For documentation on the command itself, see crinitClientTaskEnable().
 *
 * @param ctx  The crinitTaskDB_t to operate on.
 * @param res  Return pointer for response/result.
 * @param cmd  The crinitRtimCmd_t to execute, used to pass the argument list.
 *
 * @return 0 on success, -1 on error
 */
static int crinitExecRtimCmdEnable(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd);
/**
 * Internal implementation of the "disable" command on an crinitTaskDB_t.
 *
 * For documentation on the command itself, see crinitClientTaskDisable().
 *
 * @param ctx  The crinitTaskDB_t to operate on.
 * @param res  Return pointer for response/result.
 * @param cmd  The crinitRtimCmd_t to execute, used to pass the argument list.
 *
 * @return 0 on success, -1 on error
 */
static int crinitExecRtimCmdDisable(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd);
/**
 * Internal implementation of the "stop" command on an crinitTaskDB_t.
 *
 * For documentation on the command itself, see crinitClientTaskStop().
 *
 * @param ctx  The crinitTaskDB_t to operate on.
 * @param res  Return pointer for response/result.
 * @param cmd  The crinitRtimCmd_t to execute, used to pass the argument list.
 *
 * @return 0 on success, -1 on error
 */
static int crinitExecRtimCmdStop(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd);
/**
 * Internal implementation of the "kill" command on an crinitTaskDB_t.
 *
 * For documentation on the command itself, see crinitClientTaskKill().
 *
 * @param ctx  The crinitTaskDB_t to operate on.
 * @param res  Return pointer for response/result.
 * @param cmd  The crinitRtimCmd_t to execute, used to pass the argument list.
 *
 * @return 0 on success, -1 on error
 */
static int crinitExecRtimCmdKill(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd);
/**
 * Internal implementation of the "restart" command on an crinitTaskDB_t.
 *
 * For documentation on the command itself, see crinitClientTaskRestart().
 *
 * @param ctx  The crinitTaskDB_t to operate on.
 * @param res  Return pointer for response/result.
 * @param cmd  The crinitRtimCmd_t to execute, used to pass the argument list.
 *
 * @return 0 on success, -1 on error
 */
static int crinitExecRtimCmdRestart(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd);
/**
 * Internal implementation of the "notify" command on an crinitTaskDB_t.
 *
 * For documentation on the command itself, see sd_notify() and sd_notifyf().
 *
 * @param ctx  The crinitTaskDB_t to operate on.
 * @param res  Return pointer for response/result.
 * @param cmd  The crinitRtimCmd_t to execute, used to pass the argument list.
 *
 * @return 0 on success, -1 on error
 */
static int crinitExecRtimCmdNotify(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd);
/**
 * Internal implementation of the "status" command on an crinitTaskDB_t.
 *
 * For documentation on the command itself, see crinitClientTaskGetStatus().
 *
 * @param ctx  The crinitTaskDB_t to operate on.
 * @param res  Return pointer for response/result.
 * @param cmd  The crinitRtimCmd_t to execute, used to pass the argument list.
 *
 * @return 0 on success, -1 on error
 */
static int crinitExecRtimCmdStatus(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd);
/**
 * Internal implementation of the "list" command on an crinitTaskDB_t.
 *
 * For documentation on the command itself, see crinitClientGetTaskList().
 *
 * @param ctx  The crinitTaskDB_t to operate on.
 * @param res  Return pointer for response/result.
 * @param cmd  The crinitRtimCmd_t to execute, used to pass the argument list.
 *
 * @return 0 on success, -1 on error
 */
static int crinitExecRtimCmdTaskList(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd);

/**
 * Internal implementation of the version query from the client library to crinit.
 *
 * For documentation on the command itself, see crinitClientGetVersion().
 *
 * @param ctx  The crinitTaskDB_t to operate on.
 * @param res  Return pointer for response/result.
 * @param cmd  The crinitRtimCmd_t to execute, used to pass the argument list.
 *
 * @return 0 on success, -1 on error
 */
static int crinitExecRtimCmdGetVer(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd);

/**
 * Internal implementation of the "shutdown" command.
 *
 * For documentation on the command itself, see crinitClientShutdown().
 *
 * @param ctx  The currently running crinitTaskDB, needed to inhibit spawning of new processes during shutdown.
 * @param res  Return pointer for response/result.
 * @param cmd  The crinitRtimCmd to execute, used to pass the argument list.
 *
 * @return 0 on success, -1 on error
 */
static int crinitExecRtimCmdShutdown(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd);
/**
 * Shutdown thread function.
 *
 * Run as a pthread, so that crinitExecRtimCmdShutdown() may return before the actual shutdown syscall is issued and the
 * requesting process is not blocked.
 *
 * @param args  Argument pointer, see ShdnThrArgs.
 */
static void *crinitShdnThread(void *args);
/**
 * Wait function using nanosleep for the shutdown grace period.
 *
 * @param micros  Number of microseconds to wait.
 *
 * @return  0 on success, -1 on error
 */
static inline int crinitGracePeriod(unsigned long long micros);
/**
 * Prepares mounted filesystems for shutdown.
 *
 * Performs MNT_DETACH if the file systems are not the root mount. The root mount is remounted read-only if it was
 * writable before.
 *
 * @return  0 on success, -1 on error
 */
static inline int crinitFsPrepareShutdown(void);
/**
 * Generates a list of mount points that should be unmounted before shutdown/reboot.
 *
 * The resulting list will include all entries from `/proc/mounts` whose source is not `none` and which are not the root
 * node. The list will be ordered newest mount first. Memory for the list elements will be allocated and needs to be
 * freed using crinitFreeUnMountList() when no longer in use.
 *
 * Additionally, crinitGenUnMountList() checks if the root mount entry is read-only, setting \a rootfsIsRo to true in
 * that case.
 *
 * @param um          Return pointer for the UnMountList.
 * @param rootfsIsRo  Return pointer indicating if we are on a read-only rootfs.
 *
 * @return  0 on success, -1 on error
 */
static inline int crinitGenUnMountList(crinitUnMountList_t **um, bool *rootfsIsRo);
/**
 * Frees memory allocated for an crinitUnMountList_t by crinitGenUnMountList()
 *
 * @param um  The crinitUnMountList_t to free.
 */
static inline void crinitFreeUnMountList(crinitUnMountList_t *um);

int crinitParseRtimCmd(crinitRtimCmd_t *out, const char *cmdStr) {
    if (out == NULL || cmdStr == NULL) {
        crinitErrPrint("Pointer parameters must not be NULL.");
        return -1;
    }
    if (crinitRtimOpGetByOpStr(&out->op, cmdStr) == -1) {
        crinitErrPrint("Could not parse runtime command. Unknown or invalid opcode string.");
        return -1;
    }

    size_t cmdStrLen = strlen(cmdStr);
    const char *argStart = cmdStr;
    while (*argStart != CRINIT_RTIMCMD_ARGDELIM && *argStart != '\0') {
        argStart++;
    }
    // Ignore multiple CRINIT_RTIMCMD_ARGDELIM in a row.
    while (*argStart != '\0' && *(argStart + 1) == CRINIT_RTIMCMD_ARGDELIM) {
        argStart++;
    }
    const char *argEnd = &cmdStr[cmdStrLen];

    size_t argCount = 0;
    if (argStart < argEnd) {
        const char *runner = argStart;
        while (*runner != '\0') {
            if ((*runner == CRINIT_RTIMCMD_ARGDELIM) && (*(runner + 1) != '\0') &&
                (*(runner + 1) != CRINIT_RTIMCMD_ARGDELIM)) {
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
    char tokenList[2] = {CRINIT_RTIMCMD_ARGDELIM, '\0'};
    while ((token = strtok_r(start, tokenList, &strtokState)) != NULL && i < argCount) {
        start = NULL;
        out->args[i] = token;
        i++;
    }
    out->args[argCount] = NULL;
    out->argc = argCount;
    return 0;
}

int crinitRtimCmdToMsgStr(char **out, size_t *outLen, const crinitRtimCmd_t *cmd) {
    if (out == NULL || outLen == NULL || cmd == NULL) {
        crinitErrPrint("Pointer parameters must not be NULL.");
        return -1;
    }

    const char *opStr = NULL;
    if (crinitOpStrGetByRtimOp(&opStr, cmd->op) == -1) {
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
        *runner = CRINIT_RTIMCMD_ARGDELIM;
        runner++;
        runner = stpcpy(runner, cmd->args[i]);
    }
    return 0;
}

int crinitExecRtimCmd(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd) {
    if (res == NULL || cmd == NULL) {
        crinitErrPrint("Pointer parameters must not be NULL.");
        return -1;
    }
    switch (cmd->op) {
        case CRINIT_RTIMCMD_C_ADDTASK:
            if (crinitExecRtimCmdAddTask(ctx, res, cmd) == -1) {
                crinitErrPrint("Could not execute runtime command \'ADDTASK\'.");
                return -1;
            }
            return 0;
        case CRINIT_RTIMCMD_C_ADDSERIES:
            if (crinitExecRtimCmdAddSeries(ctx, res, cmd) == -1) {
                crinitErrPrint("Could not execute runtime command \'ADDSERIES\'.");
                return -1;
            }
            return 0;
        case CRINIT_RTIMCMD_C_ENABLE:
            if (crinitExecRtimCmdEnable(ctx, res, cmd) == -1) {
                crinitErrPrint("Could not execute runtime command \'ENABLE\'.");
                return -1;
            }
            return 0;
        case CRINIT_RTIMCMD_C_DISABLE:
            if (crinitExecRtimCmdDisable(ctx, res, cmd) == -1) {
                crinitErrPrint("Could not execute runtime command \'DISABLE\'.");
                return -1;
            }
            return 0;
        case CRINIT_RTIMCMD_C_STOP:
            if (crinitExecRtimCmdStop(ctx, res, cmd) == -1) {
                crinitErrPrint("Could not execute runtime command \'STOP\'.");
                return -1;
            }
            return 0;
        case CRINIT_RTIMCMD_C_KILL:
            if (crinitExecRtimCmdKill(ctx, res, cmd) == -1) {
                crinitErrPrint("Could not execute runtime command \'KILL\'.");
                return -1;
            }
            return 0;
        case CRINIT_RTIMCMD_C_RESTART:
            if (crinitExecRtimCmdRestart(ctx, res, cmd) == -1) {
                crinitErrPrint("Could not execute runtime command \'RESTART\'.");
                return -1;
            }
            return 0;
        case CRINIT_RTIMCMD_C_NOTIFY:
            if (crinitExecRtimCmdNotify(ctx, res, cmd) == -1) {
                crinitErrPrint("Could not execute runtime command \'NOTIFY\'.");
                return -1;
            }
            return 0;
        case CRINIT_RTIMCMD_C_STATUS:
            if (crinitExecRtimCmdStatus(ctx, res, cmd) == -1) {
                crinitErrPrint("Could not execute runtime command \'STATUS\'.");
                return -1;
            }
            return 0;
        case CRINIT_RTIMCMD_C_TASKLIST:
            if (crinitExecRtimCmdTaskList(ctx, res, cmd) == -1) {
                crinitErrPrint("Could not execute runtime command \'TASKLIST\'.");
                return -1;
            }
            return 0;
        case CRINIT_RTIMCMD_C_SHUTDOWN:
            if (crinitExecRtimCmdShutdown(ctx, res, cmd) == -1) {
                crinitErrPrint("Could not execute runtime command \'SHUTDOWN\'.");
                return -1;
            }
            return 0;
        case CRINIT_RTIMCMD_C_GETVER:
            if (crinitExecRtimCmdGetVer(ctx, res, cmd) == -1) {
                crinitErrPrint("Could not execute runtime command \'GETVER\'.");
                return -1;
            }
            return 0;

        case CRINIT_RTIMCMD_R_ADDTASK:
        case CRINIT_RTIMCMD_R_ADDSERIES:
        case CRINIT_RTIMCMD_R_ENABLE:
        case CRINIT_RTIMCMD_R_DISABLE:
        case CRINIT_RTIMCMD_R_STOP:
        case CRINIT_RTIMCMD_R_KILL:
        case CRINIT_RTIMCMD_R_RESTART:
        case CRINIT_RTIMCMD_R_NOTIFY:
        case CRINIT_RTIMCMD_R_STATUS:
        case CRINIT_RTIMCMD_R_TASKLIST:
        case CRINIT_RTIMCMD_R_SHUTDOWN:
        case CRINIT_RTIMCMD_R_GETVER:
        default:
            crinitErrPrint("Could not execute opcode %d. This is an unknown opcode or a response code.", cmd->op);
            return -1;
    }
    return 0;
}

int crinitBuildRtimCmd(crinitRtimCmd_t *c, crinitRtimOp_t op, size_t argc, ...) {
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

int crinitBuildRtimCmdArray(crinitRtimCmd_t *c, crinitRtimOp_t op, int argc, const char *args[]) {
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

int crinitDestroyRtimCmd(crinitRtimCmd_t *c) {
    if (c == NULL) {
        crinitErrPrint("RtimCmd pointer must not be NULL.");
        return -1;
    }
    free(c->args[0]);
    free(c->args);
    return 0;
}

static int crinitExecRtimCmdAddTask(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd) {
    if (ctx == NULL || res == NULL || cmd == NULL) {
        crinitErrPrint("Pointer parameters must not be NULL.");
    }

    crinitDbgInfoPrint("Will execute runtime command \'ADDTASK\' with following arguments:");
    for (size_t i = 0; i < cmd->argc; i++) {
        crinitDbgInfoPrint("    args[%zu] = %s", i, cmd->args[i]);
    }

    if (cmd->argc != 3) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ADDTASK, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Wrong number of arguments.");
    }

    crinitConfKvList_t *c;
    if (crinitParseConf(&c, cmd->args[0]) == -1) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ADDTASK, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Could not parse given config.");
    }
    crinitDbgInfoPrint("File \'%s\' loaded.", cmd->args[0]);

    if (strcmp(cmd->args[2], "@unchanged") != 0) {
        crinitConfKvList_t *runner = c;
        if (strcmp(cmd->args[2], "@empty") == 0) {
            while (runner != NULL) {
                if (strcmp(runner->key, CRINIT_CONFIG_KEYSTR_DEPENDS) == 0 && runner->val != NULL) {
                    runner->val[0] = '\0';
                }
                runner = runner->next;
            }
        } else {
            bool firstEncounter = true;
            while (runner != NULL) {
                if (strcmp(runner->key, CRINIT_CONFIG_KEYSTR_DEPENDS) == 0 && runner->val != NULL) {
                    if (firstEncounter) {
                        free(runner->val);
                        runner->val = strdup(cmd->args[2]);
                        if (runner->val == NULL) {
                            crinitFreeConfList(c);
                            return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ADDTASK, 2, CRINIT_RTIMCMD_RES_ERR,
                                                      "Could not set dependencies to given string.");
                        }
                        firstEncounter = false;
                    } else {
                        runner->val[0] = '\0';
                    }
                }
                runner = runner->next;
            }
            if (firstEncounter) {  // No prior DEPENDS exists in ConfKvList.
                runner = c;
                while (runner->next != NULL) {  // Find list end
                    runner = runner->next;
                }

                // Add new DEPENDS to list end
                runner->next = malloc(sizeof(*(runner->next)));
                if (runner->next == NULL) {
                    crinitFreeConfList(c);
                    return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ADDTASK, 2, CRINIT_RTIMCMD_RES_ERR,
                                              "Could not set dependencies to given string.");
                }
                runner = runner->next;
                runner->next = NULL;
                runner->key = strdup(CRINIT_CONFIG_KEYSTR_DEPENDS);
                if (runner->key == NULL) {
                    crinitFreeConfList(c);
                    return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ADDTASK, 2, CRINIT_RTIMCMD_RES_ERR,
                                              "Could not set dependencies to given string.");
                }
                runner->val = strdup(cmd->args[2]);
                if (runner->val == NULL) {
                    crinitFreeConfList(c);
                    return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ADDTASK, 2, CRINIT_RTIMCMD_RES_ERR,
                                              "Could not set dependencies to given string.");
                }
            }
        }
    }

    crinitTask_t *t = NULL;
    if (crinitTaskCreateFromConfKvList(&t, c) == -1) {
        crinitFreeConfList(c);
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ADDTASK, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Could not create task from config.");
    }
    crinitFreeConfList(c);

    crinitDbgInfoPrint("Task extracted without error.");
    bool overwrite = false;
    if (strcmp(cmd->args[1], "true") == 0) {
        overwrite = true;
    }

    if (crinitTaskDBInsert(ctx, t, overwrite) == -1) {
        crinitFreeTask(t);
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ADDTASK, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Could not insert new task into TaskDB.");
    }
    crinitFreeTask(t);
    return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ADDTASK, 1, CRINIT_RTIMCMD_RES_OK);
}

static int crinitExecRtimCmdAddSeries(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd) {
    if (ctx == NULL || res == NULL || cmd == NULL) {
        crinitErrPrint("Pointer parameters must not be NULL.");
    }

    crinitDbgInfoPrint("Will execute runtime command \'ADDSERIES\' with following arguments:");
    for (size_t i = 0; i < cmd->argc; i++) {
        crinitDbgInfoPrint("    args[%zu] = %s", i, cmd->args[i]);
    }

    if (cmd->argc != 2) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ADDSERIES, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Wrong number of arguments.");
    }

    if (!crinitIsAbsPath(cmd->args[0])) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ADDSERIES, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Path to series file must be absolute.");
    }

    if (crinitTaskDBSetSpawnInhibit(ctx, true) == -1) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ADDSERIES, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Could not inhibit process spawning to load new series file.");
    }

    crinitGlobOptStore_t *globOpts = crinitGlobOptBorrow();
    if (globOpts == NULL) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ADDSERIES, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Could not get exclusive access to global option storage.");
    }

    crinitFreeArgvArray(globOpts->tasks);
    globOpts->tasks = NULL;

    if (crinitGlobOptRemit() == -1) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ADDSERIES, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Could not release exclusive access to global option storage.");
    }

    if (crinitLoadSeriesConf(cmd->args[0]) == -1) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ADDSERIES, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Could not load series file.");
    }

    crinitFileSeries_t taskSeries;
    if (crinitLoadTasks(&taskSeries) == -1) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ADDSERIES, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Could not load crinit tasks.");
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
                crinitTaskDBSetSpawnInhibit(ctx, false);
                return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ADDSERIES, 2, CRINIT_RTIMCMD_RES_ERR,
                                          "Memory allocation error.");
            }
            memcpy(confFn, taskSeries.baseDir, prefixLen);
            confFn[prefixLen] = '/';
            memcpy(confFn + prefixLen + 1, taskSeries.fnames[n], suffixLen + 1);
            confFnAllocated = true;
        }
        crinitConfKvList_t *c;

        if (crinitParseConf(&c, confFn) == -1) {
            crinitErrPrint("Could not parse file \'%s\'.", confFn);
            if (confFnAllocated) {
                free(confFn);
            }
            crinitDestroyFileSeries(&taskSeries);
            crinitTaskDBSetSpawnInhibit(ctx, false);
            return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ADDSERIES, 2, CRINIT_RTIMCMD_RES_ERR,
                                      "Could not parse config file.");
        }
        crinitInfoPrint("File \'%s\' loaded.", confFn);
        if (confFnAllocated) {
            free(confFn);
        }

        crinitTask_t *t = NULL;
        if (crinitTaskCreateFromConfKvList(&t, c) == -1) {
            crinitFreeConfList(c);
            crinitDestroyFileSeries(&taskSeries);
            crinitTaskDBSetSpawnInhibit(ctx, false);
            return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ADDSERIES, 2, CRINIT_RTIMCMD_RES_ERR,
                                      "Could not create task from config file.");
        }

        crinitFreeConfList(c);
        if (crinitTaskDBInsert(ctx, t, overwriteTasks) == -1) {
            crinitFreeTask(t);
            crinitDestroyFileSeries(&taskSeries);
            return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ADDTASK, 2, CRINIT_RTIMCMD_RES_ERR,
                                      "Could not insert new task into TaskDB.");
        }

        crinitFreeTask(t);
    }

    crinitDestroyFileSeries(&taskSeries);
    if (crinitTaskDBSetSpawnInhibit(ctx, false) == -1) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ADDSERIES, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Could not re-enable spawning of processes.");
    }
    return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ADDSERIES, 1, CRINIT_RTIMCMD_RES_OK);
}

static int crinitExecRtimCmdEnable(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd) {
    crinitDbgInfoPrint("Will execute runtime command \'ENABLE\' with following arguments:");
    for (size_t i = 0; i < cmd->argc; i++) {
        crinitDbgInfoPrint("    args[%zu] = %s", i, cmd->args[i]);
    }
    if (cmd->argc != 1) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ENABLE, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Wrong number of arguments.");
    }
    const char depStr[] = "@ctl\0enable";
    crinitTaskDep_t tempDep = {NULL, NULL};
    tempDep.name = malloc(sizeof(depStr));
    if (tempDep.name == NULL) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ENABLE, 2, CRINIT_RTIMCMD_RES_ERR, "Memory allocation error.");
    }
    memcpy(tempDep.name, depStr, sizeof(depStr));
    tempDep.event = tempDep.name + strlen(tempDep.name) + 1;

    if (crinitTaskDBRemoveDepFromTask(ctx, &tempDep, cmd->args[0]) == -1) {
        free(tempDep.name);
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ENABLE, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Could not remove \'enable\' dependency from task.");
    }
    free(tempDep.name);
    return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_ENABLE, 1, CRINIT_RTIMCMD_RES_OK);
}

static int crinitExecRtimCmdDisable(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd) {
    crinitDbgInfoPrint("Will execute runtime command \'DISABLE\' with following arguments:");
    for (size_t i = 0; i < cmd->argc; i++) {
        crinitDbgInfoPrint("    args[%zu] = %s", i, cmd->args[i]);
    }
    if (cmd->argc != 1) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_DISABLE, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Wrong number of arguments.");
    }
    const char newDepStr[] = "@ctl\0enable";
    crinitTaskDep_t tempDep = {NULL, NULL};
    tempDep.name = malloc(sizeof(newDepStr));
    if (tempDep.name == NULL) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_DISABLE, 2, CRINIT_RTIMCMD_RES_ERR, "Memory allocation error.");
    }
    memcpy(tempDep.name, newDepStr, sizeof(newDepStr));
    tempDep.event = tempDep.name + strlen(tempDep.name) + 1;

    if (crinitTaskDBAddDepToTask(ctx, &tempDep, cmd->args[0]) == -1) {
        free(tempDep.name);
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_DISABLE, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Could not add dependency to task.");
    }
    free(tempDep.name);
    return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_DISABLE, 1, CRINIT_RTIMCMD_RES_OK);
}

static int crinitExecRtimCmdStop(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd) {
    crinitDbgInfoPrint("Will execute runtime command \'STOP\' with following arguments:");
    for (size_t i = 0; i < cmd->argc; i++) {
        crinitDbgInfoPrint("    args[%zu] = %s", i, cmd->args[i]);
    }
    if (cmd->argc != 1) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_STOP, 2, CRINIT_RTIMCMD_RES_ERR, "Wrong number of arguments.");
    }
    if (crinitSetInhibitWait(true) == -1) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_STOP, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Could not inhibit waiting for processes.");
    }
    pid_t taskPid = 0;
    if (crinitTaskDBGetTaskPID(ctx, &taskPid, cmd->args[0]) == -1) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_STOP, 2, CRINIT_RTIMCMD_RES_ERR, "Could not access task.");
    }
    if (taskPid <= 0) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_STOP, 2, CRINIT_RTIMCMD_RES_ERR, "No PID registered for task.");
    }
    if (kill(taskPid, SIGTERM) == -1) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_STOP, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Could not send SIGTERM to process.");
    }
    if (crinitSetInhibitWait(false) == -1) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_STOP, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Could not reactivate waiting for processes.");
    }
    return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_STOP, 1, CRINIT_RTIMCMD_RES_OK);
}

static int crinitExecRtimCmdKill(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd) {
    crinitDbgInfoPrint("Will execute runtime command \'KILL\' with following arguments:");
    for (size_t i = 0; i < cmd->argc; i++) {
        crinitDbgInfoPrint("    args[%zu] = %s", i, cmd->args[i]);
    }
    if (cmd->argc != 1) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_KILL, 2, CRINIT_RTIMCMD_RES_ERR, "Wrong number of arguments.");
    }

    if (crinitSetInhibitWait(true) == -1) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_STOP, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Could not inhibit waiting for processes.");
    }
    pid_t taskPid = 0;
    if (crinitTaskDBGetTaskPID(ctx, &taskPid, cmd->args[0]) == -1) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_KILL, 2, CRINIT_RTIMCMD_RES_ERR, "Could not access task.");
    }
    if (taskPid <= 0) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_KILL, 2, CRINIT_RTIMCMD_RES_ERR, "No PID registered for task.");
    }
    if (kill(taskPid, SIGKILL) == -1) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_KILL, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Could not send SIGKILL to Process.");
    }
    if (crinitSetInhibitWait(false) == -1) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_STOP, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Could not reactivate waiting for processes.");
    }
    return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_KILL, 1, CRINIT_RTIMCMD_RES_OK);
}

static int crinitExecRtimCmdRestart(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd) {
    crinitDbgInfoPrint("Will execute runtime command \'RESTART\' with following arguments:");
    for (size_t i = 0; i < cmd->argc; i++) {
        crinitDbgInfoPrint("    args[%zu] = %s", i, cmd->args[i]);
    }
    if (cmd->argc != 1) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_RESTART, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Wrong number of arguments.");
    }

    crinitTaskState_t s = 0;
    if (crinitTaskDBGetTaskState(ctx, &s, cmd->args[0]) == -1) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_RESTART, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Could not get task state from TaskDB.");
    }

    if (!(s & (CRINIT_TASK_STATE_DONE | CRINIT_TASK_STATE_FAILED))) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_RESTART, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Task is not either DONE or FAILED.");
    }

    if (crinitTaskDBSetTaskState(ctx, 0, cmd->args[0]) == -1) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_RESTART, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Could not reset task state.");
    }
    return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_RESTART, 1, CRINIT_RTIMCMD_RES_OK);
}

static int crinitExecRtimCmdNotify(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd) {
    crinitDbgInfoPrint("Will execute runtime command \'NOTIFY\' with following arguments:");
    for (size_t i = 0; i < cmd->argc; i++) {
        crinitDbgInfoPrint("    args[%zu] = %s", i, cmd->args[i]);
    }

    // Notify library will need to send task name. So process dispatch will need to set it in an environment for
    // execve.
    if (cmd->argc < 2) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_NOTIFY, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Wrong number of arguments.");
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
        if (crinitTaskDBSetTaskPID(ctx, pid, cmd->args[0]) == -1) {
            return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_NOTIFY, 2, CRINIT_RTIMCMD_RES_ERR,
                                      "Could not set main PID of task.");
        }
    }

    if (ready > 0) {
        const crinitTaskState_t s = CRINIT_TASK_STATE_RUNNING | CRINIT_TASK_STATE_NOTIFIED;
        char depEvent[] = CRINIT_TASK_EVENT_RUNNING CRINIT_TASK_EVENT_NOTIFY_SUFFIX;
        if (crinitTaskDBSetTaskState(ctx, s, cmd->args[0]) == -1) {
            return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_NOTIFY, 2, CRINIT_RTIMCMD_RES_ERR,
                                      "Could not set task state to RUNNING.");
        }

        crinitTaskDep_t dep = {cmd->args[0], depEvent};
        if (crinitTaskDBFulfillDep(ctx, &dep, NULL) == -1) {
            return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_NOTIFY, 2, CRINIT_RTIMCMD_RES_ERR,
                                      "Could not fulfill dependency \'%s:%s\'.", cmd->args[0], depEvent);
        }

        if (crinitTaskDBProvideFeatureByTaskName(ctx, cmd->args[0], s) == -1) {
            return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_NOTIFY, 2, CRINIT_RTIMCMD_RES_ERR,
                                      "Could not set features of spawned task \'%s\' as provided.", cmd->args[0]);
        }
    }

    if (stopping > 0) {
        const crinitTaskState_t s = CRINIT_TASK_STATE_DONE | CRINIT_TASK_STATE_NOTIFIED;
        char depEvent[] = CRINIT_TASK_EVENT_DONE CRINIT_TASK_EVENT_NOTIFY_SUFFIX;

        if (crinitTaskDBSetTaskState(ctx, s, cmd->args[0]) == -1) {
            return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_NOTIFY, 2, CRINIT_RTIMCMD_RES_ERR,
                                      "Could not set task state to DONE.");
        }

        const crinitTaskDep_t dep = {cmd->args[0], depEvent};
        if (crinitTaskDBFulfillDep(ctx, &dep, NULL) == -1) {
            return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_NOTIFY, 2, CRINIT_RTIMCMD_RES_ERR,
                                      "Could not fulfill dependency \'%s:%s\'.", cmd->args[0], depEvent);
        }

        if (crinitTaskDBProvideFeatureByTaskName(ctx, cmd->args[0], s) == -1) {
            return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_NOTIFY, 2, CRINIT_RTIMCMD_RES_ERR,
                                      "Could not set features of spawned task \'%s\' as provided.", cmd->args[0]);
        }
    }

    return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_NOTIFY, 1, CRINIT_RTIMCMD_RES_OK);
}

static int crinitExecRtimCmdStatus(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd) {
    crinitDbgInfoPrint("Will execute runtime command \'STATUS\' with following arguments:");
    for (size_t i = 0; i < cmd->argc; i++) {
        crinitDbgInfoPrint("    args[%zu] = %s", i, cmd->args[i]);
    }
    if (cmd->argc != 1) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_STATUS, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Wrong number of arguments.");
    }
    crinitTaskState_t s = 0;
    pid_t pid = -1;

    if (crinitTaskDBGetTaskStateAndPID(ctx, &s, &pid, cmd->args[0]) == -1) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_STATUS, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Could not get state and PID of task.");
    }

    size_t resStrLen = snprintf(NULL, 0, "%lu\n%d", s, pid) + 1;
    char *resStr = malloc(resStrLen);
    if (resStr == NULL) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_STATUS, 2, CRINIT_RTIMCMD_RES_ERR, "Memory allocation error.");
    }
    snprintf(resStr, resStrLen, "%lu\n%d", s, pid);
    char *pidStr = strchr(resStr, '\n');
    *pidStr = '\0';
    pidStr++;
    if (crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_STATUS, 3, CRINIT_RTIMCMD_RES_OK, resStr, pidStr) == -1) {
        free(resStr);
        return -1;
    }
    free(resStr);
    return 0;
}

static int crinitExecRtimCmdTaskList(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd) {
    crinitDbgInfoPrint("Will execute runtime command \'TASKLIST\' with following arguments:");
    for (size_t i = 0; i < cmd->argc; i++) {
        crinitDbgInfoPrint("    args[%zu] = %s", i, cmd->args[i]);
    }
    if (cmd->argc != 0) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_TASKLIST, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Wrong number of arguments.");
    }

    int ret = 0;
    size_t numTasks;
    char **tasks;

    if (crinitTaskDBExportTaskNamesToArray(ctx, &tasks, &numTasks) == -1) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_STATUS, 2, CRINIT_RTIMCMD_RES_ERR, "Memory allocation error.");
    }

    const char **args = malloc((numTasks + 1) * sizeof(*args));
    if (args == NULL) {
        ret = crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_STATUS, 2, CRINIT_RTIMCMD_RES_ERR, "Memory allocation error.");
        goto out;
    }

    args[0] = CRINIT_RTIMCMD_RES_OK;
    for (size_t i = 0; i < numTasks; i++) {
        args[i + 1] = tasks[i];
    }

    ret = crinitBuildRtimCmdArray(res, CRINIT_RTIMCMD_R_TASKLIST, numTasks + 1, args);

out:
    free(args);
    for (size_t i = 0; i < numTasks; i++) {
        free(tasks[i]);
    }
    free(tasks);

    return ret;
}

static int crinitExecRtimCmdGetVer(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd) {
    if (ctx == NULL || res == NULL || cmd == NULL) {
        crinitErrPrint("Pointer parameters must not be NULL");
        return -1;
    }

    crinitDbgInfoPrint("Will execute runtime command \'GETVER\'.");
    if (cmd->argc != 0) {
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_GETVER, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Wrong number of arguments.");
    }

    char major[8] = {'\0'}, minor[8] = {'\0'}, micro[8] = {'\0'};
    snprintf(major, sizeof(major) - 1, "%u", crinitVersion.major);
    snprintf(minor, sizeof(minor) - 1, "%u", crinitVersion.minor);
    snprintf(micro, sizeof(micro) - 1, "%u", crinitVersion.micro);

    return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_GETVER, 5, CRINIT_RTIMCMD_RES_OK, major, minor, micro,
                              crinitVersion.git);
}

static int crinitExecRtimCmdShutdown(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd) {
    if (ctx == NULL || res == NULL || cmd == NULL) {
        crinitErrPrint("Pointer parameters must not be NULL");
        return -1;
    }
    crinitShdnThrArgs_t *thrArgs = malloc(sizeof(crinitShdnThrArgs_t));
    if (thrArgs == NULL) {
        crinitErrnoPrint("Could not allocate memory for shutdown thread arguments.");
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_SHUTDOWN, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Memory allocation error.");
    }

    if (cmd->argc != 1) {
        free(thrArgs);
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_SHUTDOWN, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Wrong number of arguments.");
    }

    thrArgs->ctx = ctx;
    crinitShutdownCmd_t sCmd = (crinitShutdownCmd_t)strtol(cmd->args[0], NULL, 10);
    switch (sCmd) {
        case CRINIT_SHD_POWEROFF:
            thrArgs->shutdownCmd = RB_POWER_OFF;
            break;
        case CRINIT_SHD_REBOOT:
            thrArgs->shutdownCmd = RB_AUTOBOOT;
            break;
        case CRINIT_SHD_UNDEF:
        default:
            free(thrArgs);
            return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_SHUTDOWN, 2, CRINIT_RTIMCMD_RES_ERR, "Invalid argument.");
    }

    pthread_t shdnThreadRef;
    pthread_attr_t thrAttrs;
    pthread_attr_init(&thrAttrs);
    pthread_attr_setstacksize(&thrAttrs, CRINIT_RTIMCMD_SHDN_THREAD_STACK_SIZE);
    pthread_attr_setdetachstate(&thrAttrs, PTHREAD_CREATE_DETACHED);
    errno = pthread_create(&shdnThreadRef, &thrAttrs, crinitShdnThread, thrArgs);
    pthread_attr_destroy(&thrAttrs);
    if (errno != 0) {
        crinitErrnoPrint("Could not start shutdown thread");
        free(thrArgs);
        return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_SHUTDOWN, 2, CRINIT_RTIMCMD_RES_ERR,
                                  "Could not start shutdown thread.");
    }

    return crinitBuildRtimCmd(res, CRINIT_RTIMCMD_R_SHUTDOWN, 1, CRINIT_RTIMCMD_RES_OK);
}

static void *crinitShdnThread(void *args) {
    crinitShdnThrArgs_t *a = (crinitShdnThrArgs_t *)args;
    crinitTaskDB_t *ctx = a->ctx;
    int shutdownCmd = a->shutdownCmd;
    free(args);

    if (crinitTaskDBSetSpawnInhibit(ctx, true) == -1) {
        crinitErrPrint("Could not inhibit spawning of new tasks. Continuing anyway.");
    }

    unsigned long long gpMicros = CRINIT_CONFIG_DEFAULT_SHDGRACEP;
    if (crinitGlobOptGet(CRINIT_GLOBOPT_SHDGRACEP, &gpMicros) == -1) {
        gpMicros = CRINIT_CONFIG_DEFAULT_SHDGRACEP;
        crinitErrPrint("Could not read global option for shutdown grace period, using default: %lluus.", gpMicros);
    }

    kill(-1, SIGCONT);
    kill(-1, SIGTERM);
    crinitDbgInfoPrint("Sending SIGTERM to all processes.");
    if (crinitGracePeriod(gpMicros) == -1) {
        crinitErrPrint("Could not wait out the shutdown grace period, continuing anyway.");
    }
    kill(-1, SIGKILL);
    crinitDbgInfoPrint("Sending SIGKILL to all processes.");
    if (crinitFsPrepareShutdown() == -1) {
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

static inline int crinitGracePeriod(unsigned long long micros) {
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

static inline int crinitGenUnMountList(crinitUnMountList_t **ml, bool *rootfsIsRo) {
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
    crinitUnMountList_t *pList = malloc(sizeof(crinitUnMountList_t));
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
            crinitUnMountList_t *new = malloc(sizeof(crinitUnMountList_t));
            if (new == NULL) {
                crinitErrnoPrint("Could not allocate memory for list of mount points to be unmounted.");
                fclose(mountListStream);
                crinitFreeUnMountList(pList);
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

static inline void crinitFreeUnMountList(crinitUnMountList_t *ml) {
    crinitUnMountList_t *prev;
    while (ml != NULL) {
        prev = ml;
        ml = ml->next;
        free(prev);
    }
}

static inline int crinitFsPrepareShutdown(void) {
    int out = 0;
    crinitUnMountList_t *um = NULL;
    bool rootfsIsRo;

    if (crinitGenUnMountList(&um, &rootfsIsRo) == -1) {
        crinitErrPrint(
            "Could not generate list of targets to unmount. Will at least try to remount root filesystem as "
            "read-only.");
        rootfsIsRo = false;
        out = -1;
    } else {
        crinitUnMountList_t *runner = um;
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
        crinitFreeUnMountList(um);
    }
    // If it is (possibly) an rw rootfs, try remounting it ro.
    if (!rootfsIsRo && mount(NULL, "/", NULL, MS_REMOUNT | MS_RDONLY, NULL) == -1) {
        crinitErrnoPrint("Could not remount rootfs read-only, continuing anyway. Filesystem may be dirty on boot.");
        out = -1;
    }
    sync();
    return out;
}
