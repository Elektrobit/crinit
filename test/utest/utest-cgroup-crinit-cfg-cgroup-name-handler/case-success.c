// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCfgCgroupNameHandler(), successful execution.
 */
#ifdef ENABLE_CGROUP
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "confhdl.h"
#include "unit_test.h"
#include "utest-cgroup-crinit-cfg-cgroup-name-handler.h"

void crinitCfgGroupHandlerTestAlphaInputSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTask_t tgt;
    memset(&tgt, 0x00, sizeof(tgt));
    const char *val = "test.cg";
    tgt.cgroup = calloc(sizeof(*tgt.cgroup), 1);
    fprintf(stderr, "sizeof(*tgt.cgroup): %lu\n", sizeof(*tgt.cgroup));
    assert_non_null(tgt.cgroup);
    assert_int_equal(crinitCfgCgroupNameHandler(&tgt, val, CRINIT_CONFIG_TYPE_TASK), 0);
    assert_string_equal(tgt.cgroup->name, "test.cg");
    crinitDestroyTask(&tgt);
}
#endif
