/**
 * @file case-null-input.c
 * @brief Unit test for crinitConfConvToEnvSetMember() with NULL inputs.
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

void crinitConfConvToEnvSetMemberTestNullInput(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitEnvSet_t failureDummy = {NULL, 0, 0};
    crinitEnvSet_t successDummy = {NULL, 0, 0};

    const char *envConf = "VAR_NAME \"some val\"";

    assert_int_equal(crinitEnvSetInit(&successDummy, CRINIT_ENVSET_INITIAL_SIZE, CRINIT_ENVSET_SIZE_INCREMENT), 0);

    assert_int_equal(crinitConfConvToEnvSetMember(NULL, NULL), -1);
    assert_int_equal(crinitConfConvToEnvSetMember(NULL, envConf), -1);
    assert_int_equal(crinitConfConvToEnvSetMember(&successDummy, NULL), -1);
    assert_int_equal(crinitConfConvToEnvSetMember(&failureDummy, envConf), -1);

    assert_int_equal(crinitEnvSetDestroy(&successDummy), 0);
}
