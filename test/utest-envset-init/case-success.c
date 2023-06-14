/**
 * @file case-success.c
 * @brief Unit test for EBCL_envSetInit(), successful execution.
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

void EBCL_envSetInitTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    char *envp[EBCL_ENVSET_INITIAL_SIZE];

    ebcl_EnvSet_t e;

    expect_value(__wrap_calloc, num, EBCL_ENVSET_INITIAL_SIZE);
    expect_value(__wrap_calloc, size, sizeof(char *));
    will_return(__wrap_calloc, envp);

    assert_int_equal(EBCL_envSetInit(&e, EBCL_ENVSET_INITIAL_SIZE, EBCL_ENVSET_SIZE_INCREMENT), 0);

    assert_ptr_equal(e.envp, envp);
}
