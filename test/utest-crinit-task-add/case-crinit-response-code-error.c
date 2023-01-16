/**
 * @file case-crinit-response-code-error.c
 * @brief Unit test for EBCL_crinitTaskAdd() testing error handling for error code response.
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

static ebcl_RtimCmd_t *EBCL_buildRtimArgCmd;
static ebcl_RtimCmd_t *EBCL_crinitXferArgRes;
static char *EBCL_crinitXferArgResErrArgs[1] = {
    EBCL_RTIMCMD_RES_ERR
};
static ebcl_RtimCmd_t EBCL_crinitXferArgResErr = {
    .op = EBCL_RTIMCMD_R_ADDTASK,
    .argc = 1,
    .args = EBCL_crinitXferArgResErrArgs
};
static struct EBCL_storeRtimCmdArgs EBCL_crinitXferArgResContext = {
    &EBCL_crinitXferArgRes,
    &EBCL_crinitXferArgResErr,
};

void EBCL_crinitTaskAddTestCrinitResponseCodeError(void **state) {
    EBCL_PARAM_UNUSED(state);

    expect_check(__wrap_EBCL_buildRtimCmd, c, EBCL_storeRtimCmd, &EBCL_buildRtimArgCmd);
    expect_value(__wrap_EBCL_buildRtimCmd, op, EBCL_RTIMCMD_C_ADDTASK);
    expect_value(__wrap_EBCL_buildRtimCmd, argc, 3);
    expect_string(__wrap_EBCL_buildRtimCmd, vargs[0], TEST_CONFIG_FILE);
    expect_string(__wrap_EBCL_buildRtimCmd, vargs[1], "false");
    expect_string(__wrap_EBCL_buildRtimCmd, vargs[2], TEST_FORCE_DEPS);
    will_return(__wrap_EBCL_buildRtimCmd, 0);
    expect_any(__wrap_EBCL_crinitXfer, sockFile);
    expect_check(__wrap_EBCL_crinitXfer, res, EBCL_storeRtimCmdContext, &EBCL_crinitXferArgResContext);
    expect_check(__wrap_EBCL_crinitXfer, cmd, EBCL_checkRtimCmd, &EBCL_buildRtimArgCmd);
    will_return(__wrap_EBCL_crinitXfer, 0);
    expect_check(__wrap_EBCL_destroyRtimCmd, c, EBCL_checkRtimCmd, &EBCL_buildRtimArgCmd);
    will_return(__wrap_EBCL_destroyRtimCmd, 0);
    expect_check(__wrap_EBCL_destroyRtimCmd, c, EBCL_checkRtimCmd, &EBCL_crinitXferArgRes);
    will_return(__wrap_EBCL_destroyRtimCmd, 0);
    assert_int_equal(EBCL_crinitTaskAdd(TEST_CONFIG_FILE, false, TEST_FORCE_DEPS), -1);
}
