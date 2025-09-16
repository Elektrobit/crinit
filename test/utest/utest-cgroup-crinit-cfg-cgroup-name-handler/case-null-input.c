// SPDX-License-Identifier: MIT
/**
 * @file case-null-input.c
 * @brief Unit test for crinitCfgCgroupNameHandler(), input parameter is NULL.
 */
#ifdef ENABLE_CGROUP
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "confhdl.h"
#include "unit_test.h"
#include "utest-cgroup-crinit-cfg-cgroup-name-handler.h"

void crinitCfgGroupHandlerTestNullInput(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTask_t tgt;
    memset(&tgt, 0x00, sizeof(tgt));
    const char *val = NULL;
    assert_int_equal(crinitCfgCgroupNameHandler(&tgt, val, CRINIT_CONFIG_TYPE_TASK), -1);
    assert_null(tgt.cgroup);
}
#endif
