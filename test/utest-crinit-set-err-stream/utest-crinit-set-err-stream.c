/**
 * @file utest-crinit-set-err-stream.c
 * @brief Implementation of the crinitClientSetErrStream() unit test group.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "utest-crinit-set-err-stream.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitClientSetErrStream using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {cmocka_unit_test(crinitClientSetErrStreamTestSet)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
