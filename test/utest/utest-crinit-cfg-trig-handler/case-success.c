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

void crinitCfgTrigHandlerTestSuccess(void **state) {
    crinitTask_t *tgt = *state;
    const char *val = "earlysetup:wait network-dhcp:wait";
    int res = crinitCfgTrigHandler((void *)tgt, val, CRINIT_CONFIG_TYPE_TASK);
    assert_int_equal(res, 0);
    assert_int_equal(tgt->trigSize, 2);
    assert_string_equal(tgt->trig[0].name, "earlysetup");
    assert_string_equal(tgt->trig[0].event, "wait");
    assert_string_equal(tgt->trig[1].name, "network-dhcp");
    assert_string_equal(tgt->trig[1].event, "wait");
}
