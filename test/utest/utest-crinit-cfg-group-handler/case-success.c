// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCfgGroupHandler(), successful execution.
 */

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "confhdl.h"
#include "unit_test.h"
#include "utest-crinit-cfg-group-handler.h"

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

void crinitCfgGroupHandlerTestAlphaInputTwoGroupsSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTask_t tgt;
    memset(&tgt, 0x00, sizeof(tgt));
    const char *val = "disk floppy";
    will_return_count(__wrap_getgrnam_r, 0, 2);
    assert_int_equal(crinitCfgGroupHandler(&tgt, val, CRINIT_CONFIG_TYPE_TASK), 0);
    assert_int_equal(tgt.group, 42);
    assert_string_equal(tgt.groupname, "disk");
    assert_int_equal(tgt.supGroupsSize, 1);
    assert_int_equal(tgt.supGroups[0], 15);
    free(tgt.groupname);
    free(tgt.supGroups);
}

void crinitCfgGroupHandlerTestAlphaInputThreeGroupsSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTask_t tgt;
    memset(&tgt, 0x00, sizeof(tgt));
    const char *val = "nogroup disk floppy";
    will_return_count(__wrap_getgrnam_r, 0, 3);
    assert_int_equal(crinitCfgGroupHandler(&tgt, val, CRINIT_CONFIG_TYPE_TASK), 0);
    assert_int_equal(tgt.group, 65534);
    assert_string_equal(tgt.groupname, "nogroup");
    assert_int_equal(tgt.supGroupsSize, 2);
    assert_int_equal(tgt.supGroups[0], 42);
    assert_int_equal(tgt.supGroups[1], 15);
    free(tgt.groupname);
    free(tgt.supGroups);
}

void crinitCfgGroupHandlerTestNumericMultipleGroupsSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTask_t tgt;
    memset(&tgt, 0x00, sizeof(tgt));
    const char *val = "42 15";
    will_return_count(__wrap_getgrgid_r, 0, 2);
    assert_int_equal(crinitCfgGroupHandler(&tgt, val, CRINIT_CONFIG_TYPE_TASK), 0);
    assert_int_equal(tgt.group, 42);
    assert_string_equal(tgt.groupname, "disk");
    assert_int_equal(tgt.supGroupsSize, 1);
    assert_int_equal(tgt.supGroups[0], 15);
    free(tgt.groupname);
    free(tgt.supGroups);
}
