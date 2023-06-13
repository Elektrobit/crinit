/**
 * @file case-null-input.c
 * @brief Unit test for crinitEnvSetSet() with NULL inputs.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "common.h"
#include "envset.h"
#include "unit_test.h"
#include "utest-envset-set.h"

void crinitEnvSetSetTestNullInput(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitEnvSet_t failureDummy = {NULL, 0, 0};
    crinitEnvSet_t successDummy = {NULL, 0, 0};

    const char *envName = "ENV", *envVal = "val";

    assert_int_equal(crinitEnvSetInit(&successDummy, CRINIT_ENVSET_INITIAL_SIZE, CRINIT_ENVSET_SIZE_INCREMENT), 0);

    assert_int_equal(crinitEnvSetSet(NULL, NULL, NULL), -1);
    assert_int_equal(crinitEnvSetSet(NULL, NULL, envVal), -1);
    assert_int_equal(crinitEnvSetSet(NULL, envName, envVal), -1);
    assert_int_equal(crinitEnvSetSet(&successDummy, NULL, NULL), -1);
    assert_int_equal(crinitEnvSetSet(&successDummy, NULL, envVal), -1);
    assert_int_equal(crinitEnvSetSet(&successDummy, envName, NULL), -1);
    assert_int_equal(crinitEnvSetSet(&failureDummy, envName, envVal), -1);

    assert_int_equal(crinitEnvSetDestroy(&successDummy), 0);
}
