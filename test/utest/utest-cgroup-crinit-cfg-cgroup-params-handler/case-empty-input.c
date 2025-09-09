// SPDX-License-Identifier: MIT
/**
 * @file case-null-input.c
 * @brief Unit test for crinitCfgCgroupParamsHandler(), input parameter is empty.
 */

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "confhdl.h"
#include "unit_test.h"
#include "utest-cgroup-crinit-cfg-cgroup-params-handler.h"

void crinitCfgCGroupParamsHandlerTestEmptyInput(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTask_t tgt;
    memset(&tgt, 0x00, sizeof(tgt));
    const char *val = "";
    assert_int_equal(crinitCfgCgroupParamsHandler(&tgt, val, CRINIT_CONFIG_TYPE_TASK), -1);
    assert_null(tgt.cgroup);
    crinitDestroyTask(&tgt);
}
