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
    tgt.cgroup = calloc(sizeof(*tgt.cgroup), 1);
    assert_non_null(tgt.cgroup);
    tgt.cgroup->name = strdup("test.cg");
    assert_int_equal(crinitCfgCgroupParamsHandler(&tgt, val, CRINIT_CONFIG_TYPE_TASK), 0);
    assert_non_null(tgt.cgroup->config);
    assert_int_equal(tgt.cgroup->config->paramCount, 1);
    assert_string_equal(tgt.cgroup->config->param[0].filename, "key");
    assert_string_equal(tgt.cgroup->config->param[0].option, "value");
    crinitDestroyTask(&tgt);
}

void crinitCfgCGroupParamsHandlerTestTwoKeyValuesSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTask_t tgt;
    memset(&tgt, 0x00, sizeof(tgt));
    const char *val1 = "key1=value1";
    const char *val2 = "key2=value2";
    tgt.cgroup = calloc(sizeof(*tgt.cgroup), 1);
    assert_non_null(tgt.cgroup);
    tgt.cgroup->name = strdup("test.cg");
    assert_int_equal(crinitCfgCgroupParamsHandler(&tgt, val1, CRINIT_CONFIG_TYPE_TASK), 0);
    assert_non_null(tgt.cgroup->config);
    assert_int_equal(tgt.cgroup->config->paramCount, 1);
    assert_int_equal(crinitCfgCgroupParamsHandler(&tgt, val2, CRINIT_CONFIG_TYPE_TASK), 0);
    assert_non_null(tgt.cgroup->config);
    assert_int_equal(tgt.cgroup->config->paramCount, 2);
    assert_string_equal(tgt.cgroup->config->param[0].filename, "key1");
    assert_string_equal(tgt.cgroup->config->param[0].option, "value1");
    assert_string_equal(tgt.cgroup->config->param[1].filename, "key2");
    assert_string_equal(tgt.cgroup->config->param[1].option, "value2");
    crinitDestroyTask(&tgt);
}
