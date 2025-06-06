// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-cap-convert-to-bitmask.c
 * @brief Implementation of the crinitCapConvertToBitmask() unit test group.
 */

#include "utest-crinit-cap-convert-to-bitmask.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitCapConvertToBitmask() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_crinitCapConvertToBitmask),
        cmocka_unit_test(test_crinitCapConvertToBitmask_invalid_capability_name),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
