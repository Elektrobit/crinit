// SPDX-License-Identifier: MIT
/**
 * @file utest-cgroup-crinit-global-name-handler.c
 * @brief Implementation of the crinitCfgCgroupGlobalNameHandler(() unit test group.
 */

#include "utest-cgroup-crinit-global-name-handler.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitCfgCgroupGlobalNameHandler(() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitCfgCgroupGlobalNameHandlerTestAlphaInputOneValueSuccess),
        cmocka_unit_test(crinitCfgCgroupGlobalNameHandlerTestAlphaInputTwoValuesSuccess),
        cmocka_unit_test(crinitCfgCgroupGlobalNameHandlerTestNullInput),
        cmocka_unit_test(crinitCfgCgroupGlobalNameHandlerTestEmptyInput),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
