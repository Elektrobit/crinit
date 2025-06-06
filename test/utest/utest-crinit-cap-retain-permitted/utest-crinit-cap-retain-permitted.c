// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-cap-retain-permitted.c
 * @brief Implementation of the crinitCapRetainPermitted() unit test group.
 */

#include "utest-crinit-cap-retain-permitted.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitCapRetainPermitted() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_crinitCapRetainPermitted),
        cmocka_unit_test(test_crinitCapRetainPermitted_fail),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
