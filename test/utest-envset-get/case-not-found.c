/**
 * @file case-not-found.c
 * @brief Unit test for crinitEnvSetGet(), case for variable not found.
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
#include "utest-envset-get.h"

void crinitEnvSetGetTestNotFound(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitEnvSet_t e = {NULL, 0, 0};
    assert_int_equal(crinitEnvSetInit(&e, CRINIT_ENVSET_INITIAL_SIZE, CRINIT_ENVSET_SIZE_INCREMENT), 0);
    assert_int_equal(crinitEnvSetSet(&e, "VAR1", "val1"), 0);
    assert_int_equal(crinitEnvSetSet(&e, "VAR3", "val3"), 0);

    assert_null(crinitEnvSetGet(&e, "VAR2"));

    assert_int_equal(crinitEnvSetDestroy(&e), 0);
}
