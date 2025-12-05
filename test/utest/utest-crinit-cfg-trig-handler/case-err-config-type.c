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

void crinitCfgTrigHandlerTestErrConfigType(void **state) {
    crinitTask_t *tgt = *state;
    const char *val = "earlysetup:wait network-dhcp:wait";
    int res = crinitCfgTrigHandler((void *)tgt, val, CRINIT_CONFIG_TYPE_SERIES);
    assert_int_equal(res, -1);
    assert_int_equal(tgt->trigSize, 0);
    assert_ptr_equal(tgt->trig, NULL);

    res = crinitCfgTrigHandler((void *)&tgt, val, CRINIT_CONFIG_TYPE_KCMDLINE);
    assert_int_equal(res, -1);
    assert_int_equal(tgt->trigSize, 0);
    assert_ptr_equal(tgt->trig, NULL);
}
