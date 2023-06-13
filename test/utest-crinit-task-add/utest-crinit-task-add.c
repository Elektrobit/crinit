/**
 * @file utest-crinit-task-add.c
 * @brief Implementation of the crinitClientTaskAdd() unit test group.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "utest-crinit-task-add.h"

#include "rtimcmd.h"
#include "unit_test.h"

int EBCL_storeRtimCmd(const uintmax_t value, const uintmax_t context) {
    ebcl_RtimCmd_t **dest = (ebcl_RtimCmd_t **)context;
    *dest = (ebcl_RtimCmd_t *)value;
    return 1;
}

int EBCL_storeRtimCmdContext(const uintmax_t value, const uintmax_t context) {
    struct EBCL_storeRtimCmdArgs *rtimContext = (struct EBCL_storeRtimCmdArgs *)context;
    *rtimContext->ptr = (ebcl_RtimCmd_t *)value;
    **rtimContext->ptr = *rtimContext->value;
    return 1;
}

int EBCL_checkRtimCmd(const uintmax_t value, const uintmax_t context) {
    ebcl_RtimCmd_t *expected = *((ebcl_RtimCmd_t **)context);
    return (ebcl_RtimCmd_t *)value == expected;
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
