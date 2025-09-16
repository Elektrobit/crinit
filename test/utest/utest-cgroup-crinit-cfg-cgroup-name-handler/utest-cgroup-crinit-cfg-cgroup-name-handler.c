// SPDX-License-Identifier: MIT
/**
 * @file utest-cgroup-crinit-cfg-cgroup-name-handler.c
 * @brief Implementation of the crinitCfgCGroupNameHandler() unit test group.
 */
#ifdef ENABLE_CGROUP
#include "utest-cgroup-crinit-cfg-cgroup-name-handler.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitCfgCGroupNameHandler() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitCfgGroupHandlerTestAlphaInputSuccess),
        cmocka_unit_test(crinitCfgGroupHandlerTestNullInput),
        cmocka_unit_test(crinitCfgGroupHandlerTestEmptyInput),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
#endif
