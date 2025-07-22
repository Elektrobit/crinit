// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCfgCgroupParamsHandler(), successful execution.
 */

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "confhdl.h"
#include "unit_test.h"
#include "utest-cgroup-crinit-cfg-cgroup-params-handler.h"

void crinitCfgCGroupParamsHandlerTestSingleKeyValueSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTask_t tgt;
    memset(&tgt, 0x00, sizeof(tgt));
    const char *val = "key=value";
    tgt.cgroupname = strdup("test.cg");
    assert_int_equal(crinitCfgCgroupParamsHandler(&tgt, val, CRINIT_CONFIG_TYPE_TASK), 0);
    assert_non_null(tgt.cgroupConfig);
    assert_int_equal(tgt.cgroupConfig->paramCount, 1);
    assert_string_equal(tgt.cgroupConfig->param[0]->filename, "key");
    assert_string_equal(tgt.cgroupConfig->param[0]->option, "value");
    crinitDestroyTask(&tgt);
}

void crinitCfgCGroupParamsHandlerTestTwoKeyValueSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTask_t tgt;
    memset(&tgt, 0x00, sizeof(tgt));
    const char *val = "key=value \"anotherkey=another value\"";
    tgt.cgroupname = strdup("test.cg");
    assert_int_equal(crinitCfgCgroupParamsHandler(&tgt, val, CRINIT_CONFIG_TYPE_TASK), 0);
    assert_non_null(tgt.cgroupConfig);
    assert_int_equal(tgt.cgroupConfig->paramCount, 2);
    assert_string_equal(tgt.cgroupConfig->param[0]->filename, "key");
    assert_string_equal(tgt.cgroupConfig->param[0]->option, "value");
    assert_string_equal(tgt.cgroupConfig->param[1]->filename, "anotherkey");
    assert_string_equal(tgt.cgroupConfig->param[1]->option, "another value");
    crinitDestroyTask(&tgt);
}
