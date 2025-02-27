// SPDX-License-Identifier: MIT
/**
 * @file case-invalid-input.c
 * @brief Unit test for crinitCfgLauncherCmdHandler(), handling of invalid input.
 */

#include <string.h>

#include "common.h"
#include "confhdl.h"
#include "globopt.h"
#include "unit_test.h"
#include "utest-crinit-cfg-launch-cmd-handler.h"

void crinitCfgLauncherCmdHandlerTestExistingFileNotExecutable(void **state) {
    CRINIT_PARAM_UNUSED(state);

    assert_int_equal(crinitGlobOptInitDefault(), 0);
    const char *val = "/etc/passwd";
    assert_int_equal(crinitCfgLauncherCmdHandler(NULL, val, CRINIT_CONFIG_TYPE_SERIES), -1);
    crinitGlobOptDestroy();
}
