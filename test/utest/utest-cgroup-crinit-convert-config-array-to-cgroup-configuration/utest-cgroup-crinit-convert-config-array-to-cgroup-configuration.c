// SPDX-License-Identifier: MIT
/**
 * @file utest-cgroup-crinit-convert-config-array-to-cgroup-configuration.c
 * @brief Implementation of the crinitConvertConfigArrayToCGroupConfiguration() unit test group.
 */
#ifdef ENABLE_CGROUP
#include "utest-cgroup-crinit-convert-config-array-to-cgroup-configuration.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitConvertConfigArrayToCGroupConfiguration() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitCfgCGroupParamsHandlerTestSingleKeyValueSuccess),

    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
#endif
