// SPDX-License-Identifier: MIT
/**
 * @file case-empty-input.c
 * @brief Unit test for crinitCfgLauncherCmdHandler(), handling of empty input.
 */

#include <string.h>

#include "common.h"
#include "confhdl.h"
#include "globopt.h"
#include "unit_test.h"
#include "utest-crinit-cfg-launch-cmd-handler.h"

void crinitCfgLauncherCmdHandlerTestEmptyInput(void **state) {
    CRINIT_PARAM_UNUSED(state);

    assert_int_equal(crinitGlobOptInitDefault(), 0);
    const char *val = "";
    assert_int_equal(crinitCfgLauncherCmdHandler(NULL, val, CRINIT_CONFIG_TYPE_SERIES), -1);
    crinitGlobOptDestroy();
}
