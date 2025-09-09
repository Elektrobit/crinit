// SPDX-License-Identifier: MIT
/**
 * @file case-invalid-input.c
 * @brief Unit test for crinitCfgCgroupParamsHandler(), input parameter is invalid.
 */

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "confhdl.h"
#include "unit_test.h"
#include "utest-cgroup-crinit-cfg-cgroup-params-handler.h"

void crinitCfgCGroupParamsHandlerTestInvalidInputMissingDelimiter(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTask_t tgt;
    memset(&tgt, 0x00, sizeof(tgt));
    const char *val = "key value";
    tgt.cgroup = calloc(sizeof(*tgt.cgroup), 1);
    assert_non_null(tgt.cgroup);
    tgt.cgroup->name = strdup("test.cg");
    assert_int_equal(crinitCfgCgroupParamsHandler(&tgt, val, CRINIT_CONFIG_TYPE_TASK), -1);
    assert_null(tgt.cgroup->config);
    crinitDestroyTask(&tgt);
}

void crinitCfgCGroupParamsHandlerTestInvalidInputMissingCGroupName(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTask_t tgt;
    memset(&tgt, 0x00, sizeof(tgt));
    const char *val = "key=value";
    tgt.cgroup = calloc(sizeof(*tgt.cgroup), 1);
    assert_non_null(tgt.cgroup);
    assert_int_equal(crinitCfgCgroupParamsHandler(&tgt, val, CRINIT_CONFIG_TYPE_TASK), -1);
    assert_null(tgt.cgroup->config);
    crinitDestroyTask(&tgt);
}
