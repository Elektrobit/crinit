// SPDX-License-Identifier: MIT
/**
 * @file utest-cgroup-crinit-root-name-handler.c
 * @brief Implementation of the crinitCfgCgroupRootNameHandler(() unit test group.
 */
#ifdef ENABLE_CGROUP
#include "utest-cgroup-crinit-root-name-handler.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitCfgCgroupRootNameHandler(() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitCfgCgroupRootNameHandlerTestAlphaInputSuccess),
        cmocka_unit_test(crinitCfgCgroupRootNameHandlerTestNullInput),
        cmocka_unit_test(crinitCfgCgroupRootNameHandlerTestEmptyInput),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
#endif
