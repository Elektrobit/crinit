// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCreateLauncherParameters(), successful execution.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "capabilities.h"
#include "common.h"
#include "confhdl.h"
#include "globopt.h"
#include "unit_test.h"
#include "utest-crinit-create-launcher-parameters.h"

#define TRUE_CMD "/bin/true"

#define BUF_LEN 32
#ifdef ENABLE_CAPABILITIES
static char crinitCapParam[BUF_LEN];
static char *const crinitCapParamFormatStr = "--caps=%lx";
static uint64_t crinitDefaultCaps = 0;
#endif

static crinitTask_t *crinitCreateTaskWithUserAndGroup(const char *taskname, size_t supGroupCount, gid_t *supGroups) {
    crinitTask_t *tgt = NULL;
    crinitConfKvList_t name;
    crinitConfKvList_t cmd;
    crinitConfKvList_t user;
    crinitConfKvList_t group;
    memset(&name, 0x00, sizeof(name));
    memset(&cmd, 0x00, sizeof(cmd));
    memset(&user, 0x00, sizeof(user));
    memset(&group, 0x00, sizeof(group));
    name.key = "NAME";
    name.val = strdup(taskname);
    name.next = &cmd;
    cmd.key = "COMMAND";
    cmd.val = "/bin/echo -ne \"This is a test.\"";
    cmd.next = &user;
    user.key = "USER";
    user.val = "nobody";
    user.next = &group;
    group.key = "GROUP";
    group.val = "nogroup";

    assert_int_equal(crinitTaskCreateFromConfKvList(&tgt, &name), 0);
    assert_true(tgt);
    free(name.val);

    if (supGroupCount && supGroups) {
        tgt->supGroupsSize = supGroupCount;
        tgt->supGroups = supGroups;
    }

    return tgt;
}

int crinitCreateLauncherParameters(crinitTaskCmd_t *taskCmd, crinitTask_t *tCopy, char *cmd, char ***argv,
                                   char **argvBuffer);

void crinitCfgLauncherCmdHandlerTestWithOneGroupSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    char *cmd = "crintlaunch";
    char **argv = NULL;
    char *argvBuffer = NULL;

    assert_int_equal(crinitGlobOptInitDefault(), 0);
    crinitTask_t *tgt = crinitCreateTaskWithUserAndGroup("Test1", 0, NULL);
    assert_non_null(tgt);

    assert_int_equal(crinitCreateLauncherParameters(tgt->cmds, tgt, cmd, &argv, &argvBuffer), 0);
    assert_non_null(argv);
    assert_non_null(argvBuffer);

    int argvCounter = 0;
    while (argv[argvCounter] != NULL) {
        argvCounter++;
    }

#ifndef ENABLE_CAPABILITIES
    assert_int_equal(argvCounter, 7);
#endif
    assert_string_equal(argv[0], cmd);
    assert_string_equal(argv[1], "--cmd=/bin/echo");
    assert_string_equal(argv[2], "--user=65534");
    assert_string_equal(argv[3], "--group=65534");
#ifndef ENABLE_CAPABILITIES
    assert_string_equal(argv[4], "--");
    assert_string_equal(argv[5], "-ne");
    assert_string_equal(argv[6], "This is a test.");
#else
    assert_int_equal(crinitCapConvertToBitmask(&(crinitDefaultCaps), CRINIT_CONFIG_DEFAULT_DEFAULTCAPS), 0);
    const size_t capSetParamLength = snprintf(crinitCapParam, BUF_LEN, crinitCapParamFormatStr, crinitDefaultCaps);
    assert_int_not_equal(capSetParamLength, 0);
    assert_string_equal(argv[4], crinitCapParam);
    assert_string_equal(argv[5], "--");
    assert_string_equal(argv[6], "-ne");
    assert_string_equal(argv[7], "This is a test.");
#endif

    free(argv);
    free(argvBuffer);
    crinitDestroyTask(tgt);
    free(tgt);
    crinitGlobOptDestroy();
}

void crinitCfgLauncherCmdHandlerTestWithTwoGroupsSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    char *cmd = "crintlaunch";
    char **argv = NULL;
    char *argvBuffer = NULL;

    const size_t numSupGroups = 1;

    gid_t *supGroups = calloc(numSupGroups, sizeof(*supGroups));
    assert_non_null(supGroups);
    *supGroups = 6;

    assert_int_equal(crinitGlobOptInitDefault(), 0);
    crinitTask_t *tgt = crinitCreateTaskWithUserAndGroup("Test1", numSupGroups, supGroups);
    assert_non_null(tgt);

    assert_int_equal(crinitCreateLauncherParameters(tgt->cmds, tgt, cmd, &argv, &argvBuffer), 0);
    assert_non_null(argv);
    assert_non_null(argvBuffer);

    int argvCounter = 0;
    while (argv[argvCounter] != NULL) {
        argvCounter++;
    }

#ifndef ENABLE_CAPABILITIES
    assert_int_equal(argvCounter, 7);
#endif
    assert_string_equal(argv[0], cmd);
    assert_string_equal(argv[1], "--cmd=/bin/echo");
    assert_string_equal(argv[2], "--user=65534");
    assert_string_equal(argv[3], "--group=65534,6");
#ifndef ENABLE_CAPABILITIES
    assert_string_equal(argv[4], "--");
    assert_string_equal(argv[5], "-ne");
    assert_string_equal(argv[6], "This is a test.");
#else
    assert_int_equal(crinitCapConvertToBitmask(&(crinitDefaultCaps), CRINIT_CONFIG_DEFAULT_DEFAULTCAPS), 0);
    const size_t capSetParamLength = snprintf(crinitCapParam, BUF_LEN, crinitCapParamFormatStr, crinitDefaultCaps);
    assert_int_not_equal(capSetParamLength, 0);
    assert_string_equal(argv[4], crinitCapParam);
    assert_string_equal(argv[5], "--");
    assert_string_equal(argv[6], "-ne");
    assert_string_equal(argv[7], "This is a test.");
#endif

    free(argv);
    free(argvBuffer);
    crinitDestroyTask(tgt);
    free(tgt);
    crinitGlobOptDestroy();
}

void crinitCfgLauncherCmdHandlerTestWithThreeGroupsSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    char *cmd = "crintlaunch";
    char **argv = NULL;
    char *argvBuffer = NULL;

    const size_t numSupGroups = 2;

    gid_t *supGroups = calloc(numSupGroups, sizeof(*supGroups));
    assert_non_null(supGroups);
    supGroups[0] = 6;
    supGroups[1] = 35;

    assert_int_equal(crinitGlobOptInitDefault(), 0);
    crinitTask_t *tgt = crinitCreateTaskWithUserAndGroup("Test1", numSupGroups, supGroups);
    assert_non_null(tgt);

    assert_int_equal(crinitCreateLauncherParameters(tgt->cmds, tgt, cmd, &argv, &argvBuffer), 0);
    assert_non_null(argv);
    assert_non_null(argvBuffer);

    int argvCounter = 0;
    while (argv[argvCounter] != NULL) {
        argvCounter++;
    }

#ifndef ENABLE_CAPABILITIES
    assert_int_equal(argvCounter, 7);
#else
    assert_int_equal(argvCounter, 8);
#endif
    assert_string_equal(argv[0], cmd);
    assert_string_equal(argv[1], "--cmd=/bin/echo");
    assert_string_equal(argv[2], "--user=65534");
    assert_string_equal(argv[3], "--group=65534,6,35");
#ifndef ENABLE_CAPABILITIES
    assert_string_equal(argv[4], "--");
    assert_string_equal(argv[5], "-ne");
    assert_string_equal(argv[6], "This is a test.");
#else
    assert_int_equal(crinitCapConvertToBitmask(&(crinitDefaultCaps), CRINIT_CONFIG_DEFAULT_DEFAULTCAPS), 0);
    const size_t capSetParamLength = snprintf(crinitCapParam, BUF_LEN, crinitCapParamFormatStr, crinitDefaultCaps);
    assert_int_not_equal(capSetParamLength, 0);
    assert_string_equal(argv[4], crinitCapParam);
    assert_string_equal(argv[5], "--");
    assert_string_equal(argv[6], "-ne");
    assert_string_equal(argv[7], "This is a test.");
#endif
    free(argv);
    free(argvBuffer);
    crinitDestroyTask(tgt);
    free(tgt);
    crinitGlobOptDestroy();
}
