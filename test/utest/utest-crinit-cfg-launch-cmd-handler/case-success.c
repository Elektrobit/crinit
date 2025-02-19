// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCfgLauncherCmdHandler(), successful execution.
 */

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "confhdl.h"
#include "globopt.h"
#include "unit_test.h"
#include "utest-crinit-cfg-launch-cmd-handler.h"

#define TRUE_CMD "/bin/true"

void crinitCfgLauncherCmdHandlerTestExistingExecutableSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    char *launcher = NULL;
    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitCfgLauncherCmdHandler(NULL, TRUE_CMD, CRINIT_CONFIG_TYPE_SERIES), 0);
    assert_int_equal(crinitGlobOptGet(CRINIT_GLOBOPT_LAUNCHER_CMD, &launcher), 0);
    assert_non_null(launcher);
    assert_string_equal(TRUE_CMD, launcher);
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
