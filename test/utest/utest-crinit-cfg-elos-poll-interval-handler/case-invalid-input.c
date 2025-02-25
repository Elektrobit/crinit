// SPDX-License-Identifier: MIT
/**
 * @file case-invalid-input.c
 * @brief Unit test for crinitCfgElosPollIntervalHandler(), handling of invalid input.
 */

#include <string.h>

#include "common.h"
#include "confhdl.h"
#include "globopt.h"
#include "unit_test.h"
#include "utest-crinit-cfg-elos-poll-interval-handler.h"

void crinitCfgElosPollIntervalHandlerTestInvalidInput(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *val = "this_is_not_a_number";
    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitCfgElosPollIntervalHandler(NULL, val, CRINIT_CONFIG_TYPE_SERIES), -1);
    crinitGlobOptDestroy();
}
