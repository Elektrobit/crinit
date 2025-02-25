// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCfgElosPollIntervalHandler(), successful execution.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "confhdl.h"
#include "globopt.h"
#include "unit_test.h"
#include "utest-crinit-cfg-elos-poll-interval-handler.h"

#define CONFIGURED_POLL_TIME 123456

void crinitCfgElosPollIntervalHandlerTestRuntimeSettingSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    unsigned long long interval = 0;
    char val[20];
    snprintf(val, 20, "%d", CONFIGURED_POLL_TIME);
    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitCfgElosPollIntervalHandler(NULL, val, CRINIT_CONFIG_TYPE_SERIES), 0);
    assert_int_equal(crinitGlobOptGet(CRINIT_GLOBOPT_ELOS_POLL_INTERVAL, &interval), 0);
    assert_int_equal(CONFIGURED_POLL_TIME, interval);
    crinitGlobOptDestroy();
}

void crinitCfgElosPollIntervalDefaultValue(void **state) {
    CRINIT_PARAM_UNUSED(state);

    unsigned long long interval = 0;
    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitGlobOptGet(CRINIT_GLOBOPT_ELOS_POLL_INTERVAL, &interval), 0);
    assert_int_equal(CRINIT_CONFIG_DEFAULT_ELOS_POLLING_TIME, interval);
    crinitGlobOptDestroy();
}
