// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitClientTaskAdd(), successful execution.
 */

#include "common.h"
#include "crinit-client.h"
#include "rtimcmd.h"
#include "unit_test.h"
#include "utest-crinit-task-add.h"

#define TEST_CONFIG_FILE "/test/config/file"
#define TEST_FORCE_DEPS "foo:wait"

static crinitRtimCmd_t *crinitBuildRtimArgCmd;
static crinitRtimCmd_t *crinitXferArgRes;
static char *crinitXferArgResOKArgs[1] = {CRINIT_RTIMCMD_RES_OK};
static crinitRtimCmd_t crinitXferArgResOK = {.op = CRINIT_RTIMCMD_R_ADDTASK, .argc = 1, .args = crinitXferArgResOKArgs};
static struct crinitStoreRtimCmdArgs crinitXferArgResContext = {
    &crinitXferArgRes,
    &crinitXferArgResOK,
};

void crinitClientTaskAddTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    expect_check(__wrap_crinitBuildRtimCmd, c, crinitStoreRtimCmd, &crinitBuildRtimArgCmd);
    expect_value(__wrap_crinitBuildRtimCmd, op, CRINIT_RTIMCMD_C_ADDTASK);
    expect_value(__wrap_crinitBuildRtimCmd, argc, 3);
    expect_string(__wrap_crinitBuildRtimCmd, vargs[0], TEST_CONFIG_FILE);
    expect_string(__wrap_crinitBuildRtimCmd, vargs[1], "false");
    expect_string(__wrap_crinitBuildRtimCmd, vargs[2], TEST_FORCE_DEPS);
    will_return(__wrap_crinitBuildRtimCmd, 0);
    expect_any(__wrap_crinitXfer, sockFile);
    expect_check(__wrap_crinitXfer, res, crinitStoreRtimCmdContext, &crinitXferArgResContext);
    expect_check(__wrap_crinitXfer, cmd, crinitCheckRtimCmd, &crinitBuildRtimArgCmd);
    will_return(__wrap_crinitXfer, 0);
    expect_check(__wrap_crinitDestroyRtimCmd, c, crinitCheckRtimCmd, &crinitBuildRtimArgCmd);
    will_return(__wrap_crinitDestroyRtimCmd, 0);
    expect_check(__wrap_crinitDestroyRtimCmd, c, crinitCheckRtimCmd, &crinitXferArgRes);
    will_return(__wrap_crinitDestroyRtimCmd, 0);
    assert_int_equal(crinitClientTaskAdd(TEST_CONFIG_FILE, false, TEST_FORCE_DEPS), 0);
}
