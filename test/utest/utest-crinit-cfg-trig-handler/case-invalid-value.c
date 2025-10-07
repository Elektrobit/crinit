// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCfgTrigHandler(), successful execution.
 **/

#include "common.h"
#include "confhdl.h"
#include "globopt.h"
#include "task.h"
#include "unit_test.h"
#include "utest-crinit-cfg-trig-handler.h"

void crinitCfgTrigHandlerTestInvalidValue(void **state) {
    crinitTask_t *tgt = *state;
    const char *val = "earlysetup:wait network-dhcpwait";
    int res = crinitCfgTrigHandler((void *)tgt, val, CRINIT_CONFIG_TYPE_TASK);
    assert_int_equal(res, -1);
    assert_int_not_equal(tgt->trigSize, 0);
}
