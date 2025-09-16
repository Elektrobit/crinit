// SPDX-License-Identifier: MIT
/**
 * @file utest-cgroup-crinit-global-params-handler.c
 * @brief Implementation of the crinitCfgCgroupGlobalParamsHandler(() unit test group.
 */
#ifdef ENABLE_CGROUP
#include "utest-cgroup-crinit-global-params-handler.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitCfgCgroupGlobalParamsHandler(() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitCfgCgroupGlobalParamsHandlerTestSingleKeyValueSuccess),
        cmocka_unit_test(crinitCfgCgroupGlobalParamsHandlerTestNullInput),
        cmocka_unit_test(crinitCfgCgroupGlobalParamsHandlerTestEmptyInput),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
#endif
