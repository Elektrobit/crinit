// SPDX-License-Identifier: MIT
/**
 * @file case-shutdown-with-stop-command.c
 * @brief Unit test for crinitExecRtimCmd(), successful execution.
 */

#include "rtimcmd.h"
#include "common.h"
#include "globopt.h"
#include "version.h"
#include "unit_test.h"
#include "utest-crinit-exec-rtim-cmd.h"

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static pthread_mutex_t crinitLock;
static unsigned int crinitSpawnForStopCmdCalled = 0;

static int crinitSpawnMockFunc(crinitTaskDB_t *ctx, const crinitTask_t *t, crinitDispatchThreadMode_t mode) {
    CRINIT_PARAM_UNUSED(ctx);
    CRINIT_PARAM_UNUSED(t);

    if (mode == CRINIT_DISPATCH_THREAD_MODE_STOP) {
        if ((errno = pthread_mutex_lock(&crinitLock)) == 0) {
            crinitSpawnForStopCmdCalled++;
            pthread_mutex_unlock(&crinitLock);
        }
    }

    return 0;
}

const crinitVersion_t crinitVersion = {
    .major = 0,
    .minor = 0,
    .micro = 0,
    .git = "none",
};

static crinitTask_t * crinitCreateTaskWithStopCommand(const char *taskname) {
    crinitTask_t *tgt = NULL;
    crinitConfKvList_t name;
    crinitConfKvList_t cmd;
    crinitConfKvList_t stopCmd;
    memset(&name, 0x00, sizeof(name));
    memset(&cmd, 0x00, sizeof(cmd));
    memset(&stopCmd, 0x00, sizeof(stopCmd));
    name.key = "NAME";
    name.val = strdup(taskname);
    name.next = &cmd;
    cmd.key = "COMMAND";
    cmd.val = "/bin/true";
    cmd.next = &stopCmd;
    stopCmd.key = "STOP_COMMAND";
    stopCmd.val = "/bin/true";

    assert_int_equal(crinitTaskCreateFromConfKvList(&tgt, &name), 0);
    assert_true(tgt);
    free(name.val);
    return tgt;
}

void crinitExecRtimCmdTestShutdownWithStopCommand(void **state) {
    CRINIT_PARAM_UNUSED(state);

    errno = pthread_mutex_lock(&crinitLock);
    assert_int_equal(errno, 0);
    crinitSpawnForStopCmdCalled = 0;
    pthread_mutex_unlock(&crinitLock);

    crinitTaskDB_t ctx;
    crinitRtimCmd_t rtRes;
    crinitRtimCmd_t rtCmd;

    assert_int_equal(crinitGlobOptInitDefault(), 0);

    crinitTask_t *task1 = crinitCreateTaskWithStopCommand("task1");

    crinitTask_t *taskTmp = calloc(1, sizeof(crinitTask_t));
    assert_int_equal(crinitTaskCopy(taskTmp, task1), 0);
    crinitDestroyTask(taskTmp);
    free(taskTmp);

    crinitTaskDBInit(&ctx, crinitSpawnMockFunc);

    assert_int_equal(crinitTaskDBInsert(&ctx, task1, true), 0);
    crinitDestroyTask(task1);
    free(task1);

    crinitShutdownCmd_t sCmd = CRINIT_SHD_POWEROFF;
    char sCmdStr[2] = {0};
    snprintf(sCmdStr, 2, "%d", sCmd);
    assert_int_equal(crinitBuildRtimCmd(&rtCmd, CRINIT_RTIMCMD_C_SHUTDOWN, 1, sCmdStr), 0);

    assert_int_equal(crinitExecRtimCmd(&ctx, &rtRes, &rtCmd), 0);
    crinitDestroyRtimCmd(&rtRes);
    crinitDestroyRtimCmd(&rtCmd);

    usleep(100000);
    errno = pthread_mutex_lock(&crinitLock);
    assert_int_equal(errno, 0);
    assert_int_equal(crinitSpawnForStopCmdCalled, 1);
    pthread_mutex_unlock(&crinitLock);

    crinitTaskDBDestroy(&ctx);
    crinitGlobOptDestroy();
}

void crinitExecRtimCmdTestShutdownWithTwoTasksWithStopCommand(void **state) {
    CRINIT_PARAM_UNUSED(state);

    errno = pthread_mutex_lock(&crinitLock);
    assert_int_equal(errno, 0);
    crinitSpawnForStopCmdCalled = 0;
    pthread_mutex_unlock(&crinitLock);

    crinitTaskDB_t ctx;
    crinitRtimCmd_t rtRes;
    crinitRtimCmd_t rtCmd;

    assert_int_equal(crinitGlobOptInitDefault(), 0);

    crinitTask_t *task1 = crinitCreateTaskWithStopCommand("task1");
    crinitTask_t *task2 = crinitCreateTaskWithStopCommand("task2");

    crinitTaskDBInit(&ctx, crinitSpawnMockFunc);

    assert_int_equal(crinitTaskDBInsert(&ctx, task1, true), 0);
    crinitDestroyTask(task1);
    free(task1);
    assert_int_equal(crinitTaskDBInsert(&ctx, task2, true), 0);
    crinitDestroyTask(task2);
    free(task2);

    crinitShutdownCmd_t sCmd = CRINIT_SHD_POWEROFF;
    char sCmdStr[2] = {0};
    snprintf(sCmdStr, 2, "%d", sCmd);
    assert_int_equal(crinitBuildRtimCmd(&rtCmd, CRINIT_RTIMCMD_C_SHUTDOWN, 1, sCmdStr), 0);

    assert_int_equal(crinitExecRtimCmd(&ctx, &rtRes, &rtCmd), 0);
    crinitDestroyRtimCmd(&rtRes);
    crinitDestroyRtimCmd(&rtCmd);

    usleep(100000);
    errno = pthread_mutex_lock(&crinitLock);
    assert_int_equal(errno, 0);
    assert_int_equal(crinitSpawnForStopCmdCalled, 2);
    pthread_mutex_unlock(&crinitLock);

    crinitTaskDBDestroy(&ctx);
    crinitGlobOptDestroy();
}
