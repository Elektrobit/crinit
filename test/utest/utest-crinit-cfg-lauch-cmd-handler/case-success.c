// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCfgLauncherCmdHandler(), successful execution.
 */

#include "confhdl.h"
#include "common.h"
#include "globopt.h"
#include "unit_test.h"
#include "utest-crinit-cfg-launch-cmd-handler.h"

#include <stdlib.h>
#include <string.h>

void crinitCfgLauncherCmdHandlerTestExistingExecutableSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    char *launcher = NULL;
    assert_int_equal(crinitGlobOptInitDefault(), 0);
    const char *val = "/usr/bin/true";
    assert_int_equal(crinitCfgLauncherCmdHandler(NULL, val, CRINIT_CONFIG_TYPE_SERIES), 0);
    assert_int_equal(crinitGlobOptGet(CRINIT_GLOBOPT_LAUNCHER_CMD, &launcher), 0);
    assert_non_null(launcher);
    assert_string_equal("/usr/bin/true", launcher);
    crinitGlobOptDestroy();
    free(launcher);
}

void crinitCfgLauncherCmdDefaultValue(void **state) {
    CRINIT_PARAM_UNUSED(state);

    char *launcher = NULL;
    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitGlobOptGet(CRINIT_GLOBOPT_LAUNCHER_CMD, &launcher), 0);
    assert_non_null(launcher);
    assert_string_equal("/usr/bin/crinit-launch", launcher);
    crinitGlobOptDestroy();
    free(launcher);
}
