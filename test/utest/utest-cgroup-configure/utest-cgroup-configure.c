// SPDX-License-Identifier: MIT
/**
 * @file utest-cgroup-configure.c
 * @brief Implementation of the crinitCGroupConfigure() unit test group.
 */
#ifdef ENABLE_CGROUP
#include "utest-cgroup-configure.h"

#include <sys/types.h>

#include "common.h"
#include "unit_test.h"

/**
 * Runs the unit test group for crinitCGroupConfigure using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        // clang-format off
        // Rationale: unreadable output of clang-format
        cmocka_unit_test(crinitCGroupConfigureTestSuccess),
        cmocka_unit_test(crinitCGroupConfigureTestSuccessParent),
        cmocka_unit_test(crinitCGroupConfigureTestWrongInput),
        cmocka_unit_test(crinitCGroupConfigureTestOpenFail),
        cmocka_unit_test(crinitCGroupConfigureTestOpenatFailFirst),
        cmocka_unit_test(crinitCGroupConfigureTestOpenatFailSecond),
        cmocka_unit_test(crinitCGroupConfigureTestOpenatFailThird),
        cmocka_unit_test(crinitCGroupConfigureTestWritevFail),
        cmocka_unit_test(crinitCGroupConfigureTestMkdirFail),
        // clang-format on
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
#endif
