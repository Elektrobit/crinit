// SPDX-License-Identifier: MIT
/**
 * @file case-null-input.c
 * @brief Unit test for crinitCfgGroupHandler(), handling of null pointer input.
 */

#include <string.h>

#include "common.h"
#include "confhdl.h"
#include "unit_test.h"
#include "utest-crinit-cfg-group-handler.h"

void crinitCfgGroupHandlerTestNullInput(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTask_t tgt;
    memset(&tgt, 0x00, sizeof(tgt));
    const char *val = NULL;
    assert_int_equal(crinitCfgGroupHandler(&tgt, val, CRINIT_CONFIG_TYPE_TASK), -1);
    assert_int_equal(tgt.group, 0);
}
