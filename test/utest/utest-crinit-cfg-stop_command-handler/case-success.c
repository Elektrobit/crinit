// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCfgStopCmdHandler(), successful execution.
 */

#include "confhdl.h"
#include "common.h"
#include "unit_test.h"
#include "utest-crinit-cfg-stop_command-handler.h"

#include <stdlib.h>
#include <string.h>

void crinitCfgStopCommandHandlerTestSingleStopCommandSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTask_t tgt;
    memset(&tgt, 0x00, sizeof(tgt));
    const char *val = "/bin/true";
    assert_int_equal(crinitCfgStopCmdHandler(&tgt, val, CRINIT_CONFIG_TYPE_TASK), 0);
    assert_int_equal(tgt.stopCmdsSize, 1);
    assert_string_equal(tgt.stopCmds[0].argv[0], "/bin/true");
    crinitDestroyTask(&tgt);
}

void crinitCfgStopCommandHandlerTestSingleStopCommandWithParameterSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTask_t tgt;
    memset(&tgt, 0x00, sizeof(tgt));
    const char *val = "/usr/bin/echo \"foo bar\"";
    assert_int_equal(crinitCfgStopCmdHandler(&tgt, val, CRINIT_CONFIG_TYPE_TASK), 0);
    assert_int_equal(tgt.stopCmdsSize, 1);
    assert_int_equal(tgt.stopCmds[0].argc, 2);
    assert_string_equal(tgt.stopCmds[0].argv[0], "/usr/bin/echo");
    assert_string_equal(tgt.stopCmds[0].argv[1], "foo bar");
    crinitDestroyTask(&tgt);
}
