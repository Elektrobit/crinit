// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-task-add.c
 * @brief Implementation of the crinitClientTaskAdd() unit test group.
 */

#include "utest-crinit-task-add.h"

#include "rtimcmd.h"
#include "unit_test.h"

int crinitStoreRtimCmd(const uintmax_t value, const uintmax_t context) {
    crinitRtimCmd_t **dest = (crinitRtimCmd_t **)context;
    *dest = (crinitRtimCmd_t *)value;
    return 1;
}

int crinitStoreRtimCmdContext(const uintmax_t value, const uintmax_t context) {
    struct crinitStoreRtimCmdArgs *rtimContext = (struct crinitStoreRtimCmdArgs *)context;
    *rtimContext->ptr = (crinitRtimCmd_t *)value;
    **rtimContext->ptr = *rtimContext->value;
    return 1;
}

int crinitCheckRtimCmd(const uintmax_t value, const uintmax_t context) {
    crinitRtimCmd_t *expected = *((crinitRtimCmd_t **)context);
    return (crinitRtimCmd_t *)value == expected;
}

/**
 * Runs the unit test group for crinitClientTaskAdd using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crinitClientTaskAddTestSuccess),
        cmocka_unit_test(crinitClientTaskAddTestConfPathNull),
        cmocka_unit_test(crinitClientTaskAddTestForceDepsNull),
        cmocka_unit_test(crinitClientTaskAddTestForceDepsEmpty),
        cmocka_unit_test(crinitClientTaskAddTestOverwriteBoolToString),
        cmocka_unit_test(crinitClientTaskAddTestBuildRtimCmdError),
        cmocka_unit_test(crinitClientTaskAddTestCrinitXferError),
        cmocka_unit_test(crinitClientTaskAddTestCrinitResponseCodeError),
        cmocka_unit_test(crinitClientTaskAddTestCrinitResponseCmdError),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
