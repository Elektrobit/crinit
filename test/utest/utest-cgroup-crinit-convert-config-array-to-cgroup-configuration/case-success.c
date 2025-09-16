// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitConvertConfigArrayToCGroupConfiguration(), successful execution.
 */
#ifdef ENABLE_CGROUP
#include <stdlib.h>
#include <string.h>

#include "cgroup.h"
#include "common.h"
#include "unit_test.h"
#include "utest-cgroup-crinit-convert-config-array-to-cgroup-configuration.h"

void crinitCfgCGroupParamsHandlerTestSingleKeyValueSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    /*crinitTask_t tgt;
    memset(&tgt, 0x00, sizeof(tgt));
    const char *val = "key=value";
    tgt.cgroupname = "test.cg";
    assert_int_equal(crinitCfgCGroupParamsHandler(&tgt, val, CRINIT_CONFIG_TYPE_TASK), 0);
    assert_int_equal(tgt.cgroupParamsSize, 1);
    assert_string_equal(tgt.cgroupParams[0], "key=value");
    free(tgt.cgroupParams[0]);
    free(tgt.cgroupParams);*/
}
#endif
