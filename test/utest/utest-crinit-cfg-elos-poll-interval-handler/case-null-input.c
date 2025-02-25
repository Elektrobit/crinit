// SPDX-License-Identifier: MIT
/**
 * @file case-null-input.c
 * @brief Unit test for crinitCfgElosEventPollIntervalHandler(), handling of null pointer input.
 */

#include <string.h>

#include "common.h"
#include "confhdl.h"
#include "globopt.h"
#include "unit_test.h"
#include "utest-crinit-cfg-elos-poll-interval-handler.h"

void crinitCfgElosPollIntervalHandlerTestNullInput(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *val = NULL;
    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitCfgElosEventPollIntervalHandler(NULL, val, CRINIT_CONFIG_TYPE_SERIES), -1);
    crinitGlobOptDestroy();
}
