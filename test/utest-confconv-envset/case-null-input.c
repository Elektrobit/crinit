/**
 * @file case-null-input.c
 * @brief Unit test for EBCL_confConvToEnvSetMember() with NULL inputs.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "common.h"
#include "confconv.h"
#include "envset.h"
#include "unit_test.h"
#include "utest-confconv-envset.h"

void EBCL_confConvToEnvSetMemberTestNullInput(void **state) {
    EBCL_PARAM_UNUSED(state);

    ebcl_EnvSet_t failureDummy = {NULL, 0, 0};
    ebcl_EnvSet_t successDummy = {NULL, 0, 0};

    const char *envConf = "VAR_NAME \"some val\"";

    assert_int_equal(EBCL_envSetInit(&successDummy, EBCL_ENVSET_INITIAL_SIZE, EBCL_ENVSET_SIZE_INCREMENT), 0);

    assert_int_equal(EBCL_confConvToEnvSetMember(NULL, NULL), -1);
    assert_int_equal(EBCL_confConvToEnvSetMember(NULL, envConf), -1);
    assert_int_equal(EBCL_confConvToEnvSetMember(&successDummy, NULL), -1);
    assert_int_equal(EBCL_confConvToEnvSetMember(&failureDummy, envConf), -1);

    assert_int_equal(EBCL_envSetDestroy(&successDummy), 0);
}
