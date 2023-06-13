/**
 * @file case-success.c
 * @brief Unit test for crinitEnvSetInit(), successful execution.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "common.h"
#include "envset.h"
#include "string.h"
#include "unit_test.h"
#include "utest-envset-init.h"

void crinitEnvSetInitTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    char *envp[CRINIT_ENVSET_INITIAL_SIZE];

    crinitEnvSet_t e;

    expect_value(__wrap_calloc, num, CRINIT_ENVSET_INITIAL_SIZE);
    expect_value(__wrap_calloc, size, sizeof(char *));
    will_return(__wrap_calloc, envp);

    assert_int_equal(crinitEnvSetInit(&e, CRINIT_ENVSET_INITIAL_SIZE, CRINIT_ENVSET_SIZE_INCREMENT), 0);

    assert_ptr_equal(e.envp, envp);
}
