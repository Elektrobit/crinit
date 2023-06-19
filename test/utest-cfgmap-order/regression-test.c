/**
 * @file regression-test.c
 * @brief Implementation of a regression test for the order/size of crinitCfgMap.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include <string.h>

#include "common.h"
#include "confmap.h"
#include "unit_test.h"
#include "utest-cfgmap-order.h"

void crinitCfgMapRegressionTest(void **state) {
    CRINIT_PARAM_UNUSED(state);

    // Check size of crinitCfgMap.
    assert_int_equal(crinitCfgMapSize, CRINIT_CONFIGS_SIZE);

    // Check order
    for (size_t i = 1; i < crinitCfgMapSize; i++) {
        // We are correctly ordered if each following configKey is lexicographically larger than the one before.
        assert_true(strcmp(crinitCfgMap[i].configKey, crinitCfgMap[i - 1].configKey) > 0);
    }
}
