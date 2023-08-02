// SPDX-License-Identifier: MIT
/**
 * @file case-build-rtim-cmd-error.c
 * @brief Unit test for crinitClientTaskAdd() testing error handling for crinitBuildRtimCmd().
 */

#include "common.h"
#include "crinit-client.h"
#include "rtimcmd.h"
#include "unit_test.h"
#include "utest-crinit-task-add.h"

#define TEST_CONFIG_FILE "/test/config/file"
#define TEST_FORCE_DEPS "foo:wait"

static crinitRtimCmd_t *crinitBuildRtimArgCmd;

void crinitClientTaskAddTestBuildRtimCmdError(void **state) {
    CRINIT_PARAM_UNUSED(state);

    expect_check(__wrap_crinitBuildRtimCmd, c, crinitStoreRtimCmd, &crinitBuildRtimArgCmd);
    expect_value(__wrap_crinitBuildRtimCmd, op, CRINIT_RTIMCMD_C_ADDTASK);
    expect_value(__wrap_crinitBuildRtimCmd, argc, 3);
    expect_string(__wrap_crinitBuildRtimCmd, vargs[0], TEST_CONFIG_FILE);
    expect_string(__wrap_crinitBuildRtimCmd, vargs[1], "false");
    expect_string(__wrap_crinitBuildRtimCmd, vargs[2], TEST_FORCE_DEPS);
    will_return(__wrap_crinitBuildRtimCmd, -1);
    assert_int_equal(crinitClientTaskAdd(TEST_CONFIG_FILE, false, TEST_FORCE_DEPS), -1);
}
