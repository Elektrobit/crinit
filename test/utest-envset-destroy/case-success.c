/**
 * @file case-success.c
 * @brief Unit test for EBCL_envSetDestroy(), successful execution.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include <stdio.h>

#include "common.h"
#include "envset.h"
#include "string.h"
#include "unit_test.h"
#include "utest-envset-destroy.h"

#define EBCL_DUMMY_INITIALIZED_ELEMENTS (EBCL_ENVSET_INITIAL_SIZE / 2uL)

void EBCL_envSetDestroyTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    char *envp[EBCL_ENVSET_INITIAL_SIZE];
    for (size_t i = 0; i < EBCL_ENVSET_INITIAL_SIZE; i++) {
        envp[i] = (i < EBCL_DUMMY_INITIALIZED_ELEMENTS) ? (char *)0xdeadc0de : NULL;
    }
    ebcl_EnvSet_t e = {&envp[0], EBCL_ENVSET_INITIAL_SIZE, EBCL_ENVSET_SIZE_INCREMENT};

    expect_value_count(__wrap_free, ptr, (char *)0xdeadc0de, EBCL_DUMMY_INITIALIZED_ELEMENTS);
    expect_value(__wrap_free, ptr, &envp[0]);

    assert_int_equal(EBCL_envSetDestroy(&e), 0);

    assert_ptr_equal(e.envp, NULL);
    assert_int_equal(e.allocSz, 0);
    assert_int_equal(e.allocInc, 0);
}
