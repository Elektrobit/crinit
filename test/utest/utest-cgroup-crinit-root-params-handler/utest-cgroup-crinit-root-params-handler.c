// SPDX-License-Identifier: MIT
/**
 * @file utest-cgroup-crinit-root-params-handler.c
 * @brief Implementation of the crinitCfgCgroupRootParamsHandler(() unit test group.
 */
#ifdef ENABLE_CGROUP
#include "utest-cgroup-crinit-root-params-handler.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitCfgCgroupRootParamsHandler(() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitCfgCgroupRootParamsHandlerTestSingleKeyValueSuccess),
        cmocka_unit_test(crinitCfgCgroupRootParamsHandlerTestNullInput),
        cmocka_unit_test(crinitCfgCgroupRootParamsHandlerTestEmptyInput),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
#endif
