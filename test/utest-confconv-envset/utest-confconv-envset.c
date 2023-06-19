/**
 * @file utest-confconv-envset.c
 * @brief Implementation of the crinitConfConvToEnvSetMember() unit test group.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "utest-confconv-envset.h"

#include "unit_test.h"

/**
 * Runs the unit test group for crinitConfConvToEnvSetMember() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitConfConvToEnvSetMemberTestSuccess),
        cmocka_unit_test(crinitConfConvToEnvSetMemberTestWrongInput),
        cmocka_unit_test(crinitConfConvToEnvSetMemberTestNullInput),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
