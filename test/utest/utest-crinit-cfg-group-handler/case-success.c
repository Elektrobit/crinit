// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCfgGroupHandler(), successful execution.
 */

#include "confhdl.h"
#include "common.h"
#include "unit_test.h"
#include "utest-crinit-cfg-group-handler.h"

#include <stdlib.h>
#include <string.h>

void crinitCfgGroupHandlerTestNumericSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTask_t tgt;
    memset(&tgt, 0x00, sizeof(tgt));
    const char *val = "42";
    will_return(__wrap_getgrgid_r, 0);
    assert_int_equal(crinitCfgGroupHandler(&tgt, val, CRINIT_CONFIG_TYPE_TASK), 0);
    assert_int_equal(tgt.group, 42);
    assert_string_equal(tgt.groupname, "disk");
    free(tgt.groupname);
}

void crinitCfgGroupHandlerTestAlphaInputSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTask_t tgt;
    memset(&tgt, 0x00, sizeof(tgt));
    const char *val = "disk";
    will_return(__wrap_getgrnam_r, 0);
    assert_int_equal(crinitCfgGroupHandler(&tgt, val, CRINIT_CONFIG_TYPE_TASK), 0);
    assert_int_equal(tgt.group, 42);
    assert_string_equal(tgt.groupname, "disk");
    free(tgt.groupname);
}
