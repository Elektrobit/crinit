// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCfgUserHandler(), successful execution.
 */

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "confhdl.h"
#include "unit_test.h"
#include "utest-crinit-cfg-user-handler.h"

void crinitCfgUserHandlerTestNumericSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTask_t tgt;
    memset(&tgt, 0x00, sizeof(tgt));
    const char *val = "42";
    will_return(__wrap_getpwuid_r, 0);
    assert_int_equal(crinitCfgUserHandler(&tgt, val, CRINIT_CONFIG_TYPE_TASK), 0);
    assert_int_equal(tgt.user, 42);
    assert_string_equal(tgt.username, "www-run");
    free(tgt.username);
}

void crinitCfgUserHandlerTestAlphaInputSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTask_t tgt;
    memset(&tgt, 0x00, sizeof(tgt));
    const char *val = "www-run";
    will_return(__wrap_getpwnam_r, 0);
    assert_int_equal(crinitCfgUserHandler(&tgt, val, CRINIT_CONFIG_TYPE_TASK), 0);
    assert_int_equal(tgt.user, 42);
    assert_string_equal(tgt.username, "www-run");
    free(tgt.username);
}
