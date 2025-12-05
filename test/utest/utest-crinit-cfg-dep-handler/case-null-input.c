// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCfgDepHandler(), successful execution.
 **/

#include "common.h"
#include "confhdl.h"
#include "globopt.h"
#include "task.h"
#include "unit_test.h"
#include "utest-crinit-cfg-dep-handler.h"

void crinitCfgDepHandlerTestNullInput(void **state) {
    crinitTask_t *tgt = *state;
    const char *val = "earlysetup:wait network-dhcp:wait";

    int res = crinitCfgDepHandler((void *)tgt, NULL, CRINIT_CONFIG_TYPE_TASK);
    assert_int_equal(res, -1);
    assert_int_equal(tgt->depsSize, 0);
    assert_ptr_equal(tgt->deps, NULL);

    res = crinitCfgDepHandler(NULL, val, CRINIT_CONFIG_TYPE_TASK);
    assert_int_equal(res, -1);

    res = crinitCfgDepHandler(NULL, NULL, CRINIT_CONFIG_TYPE_TASK);
    assert_int_equal(res, -1);
}
