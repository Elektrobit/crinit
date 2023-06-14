/**
 * @file regression-test.c
 * @brief Implementation of a regression test for the order/size of EBCL_cfgMap.
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

void EBCL_cfgMapRegressionTest(void **state) {
    CRINIT_PARAM_UNUSED(state);

    // Check size of EBCL_cfgMap.
    assert_int_equal(EBCL_cfgMapSize, EBCL_CONFIGS_SIZE);

    // Check order
    for (size_t i = 1; i < EBCL_cfgMapSize; i++) {
        // We are correctly ordered if each following configKey is lexicographically larger than the one before.
        assert_true(strcmp(EBCL_cfgMap[i].configKey, EBCL_cfgMap[i - 1].configKey) > 0);
    }
}
