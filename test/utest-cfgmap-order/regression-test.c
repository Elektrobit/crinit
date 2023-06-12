/**
 * @file regression-test.c
 * @brief Implementation of a regression test for the order and completeness of crinit{Task,Series}CfgMap.
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
#include "logio.h"
#include "unit_test.h"
#include "utest-cfgmap-order.h"

void crinitCfgMapRegressionTest(void **state) {
    CRINIT_PARAM_UNUSED(state);

    bool checkArr[CRINIT_CONFIGS_SIZE] = {false};
    checkArr[crinitTaskCfgMap[0].config] = true;
    // Check order of task config mappings.
    for (size_t i = 1; i < crinitTaskCfgMapSize; i++) {
        // We are correctly ordered if each following configKey is lexicographically larger than the one before.
        assert_true(strcmp(crinitTaskCfgMap[i].configKey, crinitTaskCfgMap[i - 1].configKey) > 0);
        checkArr[crinitTaskCfgMap[i].config] = true;
    }

    checkArr[crinitSeriesCfgMap[0].config] = true;
    // Check order of series config mappings.
    for (size_t i = 1; i < crinitSeriesCfgMapSize; i++) {
        // We are correctly ordered if each following configKey is lexicographically larger than the one before.
        assert_true(strcmp(crinitSeriesCfgMap[i].configKey, crinitSeriesCfgMap[i - 1].configKey) > 0);
        checkArr[crinitSeriesCfgMap[i].config] = true;
    }

    // Check if all crinitConfig_t have a mapping
    for (size_t i = 0; i < crinitNumElements(checkArr); i++) {
        assert_true(checkArr[i]);
    }
}
