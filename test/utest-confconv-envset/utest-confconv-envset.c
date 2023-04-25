/**
 * @file utest-confconv-envset.c
 * @brief Implementation of the EBCL_confConvToEnvSetMember() unit test group.
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
 * Runs the unit test group for EBCL_confConvToEnvSetMember() using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(EBCL_confConvToEnvSetMemberTestSuccess),
        cmocka_unit_test(EBCL_confConvToEnvSetMemberTestWrongInput),
        cmocka_unit_test(EBCL_confConvToEnvSetMemberTestNullInput),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
