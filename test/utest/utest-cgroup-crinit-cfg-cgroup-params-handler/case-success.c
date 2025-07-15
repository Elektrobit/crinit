// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCfgCGroupParamsHandler(), successful execution.
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
    tgt.cgroupname = "test.cg";
    assert_int_equal(crinitCfgCGroupParamsHandler(&tgt, val, CRINIT_CONFIG_TYPE_TASK), 0);
    assert_int_equal(tgt.cgroupParamsSize, 1);
    assert_string_equal(tgt.cgroupParams[0], "key=value");
    free(tgt.cgroupParams[0]);
    free(tgt.cgroupParams);
}

void crinitCfgCGroupParamsHandlerTestTwoKeyValueSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTask_t tgt;
    memset(&tgt, 0x00, sizeof(tgt));
    const char *val = "key=value \"anotherkey=another value\"";
    tgt.cgroupname = "test.cg";
    assert_int_equal(crinitCfgCGroupParamsHandler(&tgt, val, CRINIT_CONFIG_TYPE_TASK), 0);
    assert_int_equal(tgt.cgroupParamsSize, 2);
    assert_string_equal(tgt.cgroupParams[0], "key=value");
    assert_string_equal(tgt.cgroupParams[1], "anotherkey=another value");
    free(tgt.cgroupParams[0]);
    free(tgt.cgroupParams[1]);
    free(tgt.cgroupParams);
}
