/**
 * @file utest-crinit-task-add.c
 * @brief Implementation of the EBCL_crinitTaskAdd() unit test group.
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
 * Runs the unit test group for EBCL_crinitTaskAdd using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(EBCL_crinitTaskAddTestSuccess),
        cmocka_unit_test(EBCL_crinitTaskAddTestConfPathNull),
        cmocka_unit_test(EBCL_crinitTaskAddTestForceDepsNull),
        cmocka_unit_test(EBCL_crinitTaskAddTestForceDepsEmpty),
        cmocka_unit_test(EBCL_crinitTaskAddTestOverwriteBoolToString),
        cmocka_unit_test(EBCL_crinitTaskAddTestBuildRtimCmdError),
        cmocka_unit_test(EBCL_crinitTaskAddTestCrinitXferError),
        cmocka_unit_test(EBCL_crinitTaskAddTestCrinitResponseCodeError),
        cmocka_unit_test(EBCL_crinitTaskAddTestCrinitResponseCmdError),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
