// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-set-info-stream.c
 * @brief Implementation of the crinitClientSetInfoStream() unit test group.
 */

#include "utest-crinit-set-info-stream.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitClientSetInfoStream using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {cmocka_unit_test(crinitClientSetInfoStreamTestSet)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
