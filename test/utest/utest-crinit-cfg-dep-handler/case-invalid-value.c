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

void crinitCfgDepHandlerTestInvalidValue(void **state) {
    crinitTask_t *tgt = *state;
    const char *val = "earlysetupwait network-dhcpwait";
    int res = crinitCfgDepHandler((void *)tgt, val, CRINIT_CONFIG_TYPE_TASK);
    assert_int_equal(res, -1);
}
