/**
 * @file case-success.c
 * @brief Unit test for crinitClientTaskAdd() testing error handling for crinitXfer().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "common.h"
#include "crinit-client.h"
#include "rtimcmd.h"
#include "unit_test.h"
#include "utest-crinit-task-add.h"

#define TEST_CONFIG_FILE "/test/config/file"
#define TEST_FORCE_DEPS "foo:wait"

static crinitRtimCmd_t *EBCL_buildRtimArgCmd;

void crinitClientTaskAddTestCrinitXferError(void **state) {
    CRINIT_PARAM_UNUSED(state);

    expect_check(__wrap_crinitBuildRtimCmd, c, EBCL_storeRtimCmd, &EBCL_buildRtimArgCmd);
    expect_value(__wrap_crinitBuildRtimCmd, op, CRINIT_RTIMCMD_C_ADDTASK);
    expect_value(__wrap_crinitBuildRtimCmd, argc, 3);
    expect_string(__wrap_crinitBuildRtimCmd, vargs[0], TEST_CONFIG_FILE);
    expect_string(__wrap_crinitBuildRtimCmd, vargs[1], "false");
    expect_string(__wrap_crinitBuildRtimCmd, vargs[2], TEST_FORCE_DEPS);
    will_return(__wrap_crinitBuildRtimCmd, 0);
    expect_any(__wrap_crinitXfer, sockFile);
    expect_any(__wrap_crinitXfer, res);
    expect_check(__wrap_crinitXfer, cmd, EBCL_checkRtimCmd, &EBCL_buildRtimArgCmd);
    will_return(__wrap_crinitXfer, -1);
    expect_check(__wrap_crinitDestroyRtimCmd, c, EBCL_checkRtimCmd, &EBCL_buildRtimArgCmd);
    will_return(__wrap_crinitDestroyRtimCmd, 0);
    assert_int_equal(crinitClientTaskAdd(TEST_CONFIG_FILE, false, TEST_FORCE_DEPS), -1);
}
