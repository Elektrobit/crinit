// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCfgUserHandler(), successful execution.
 */

#include "confhdl.h"
#include "common.h"
#include "unit_test.h"
#include "utest-crinit-cfg-user-handler.h"

#include <string.h>

void crinitCfgUserHandlerTestNumericSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTask_t tgt;
    memset(&tgt, 0x00, sizeof(tgt));
    const char *val = "42";
    assert_int_equal(crinitCfgUserHandler(&tgt, val, CRINIT_CONFIG_TYPE_TASK), 0);
    assert_int_equal(tgt.user, 42);
}
