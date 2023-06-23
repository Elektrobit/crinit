/**
 * @file case-success.c
 * @brief Unit test for crinitEnvSetDestroy(), successful execution.
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

#define CRINIT_DUMMY_INITIALIZED_ELEMENTS (CRINIT_ENVSET_INITIAL_SIZE / 2uL)

void crinitEnvSetDestroyTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    char *envp[CRINIT_ENVSET_INITIAL_SIZE];
    for (size_t i = 0; i < CRINIT_ENVSET_INITIAL_SIZE; i++) {
        envp[i] = (i < CRINIT_DUMMY_INITIALIZED_ELEMENTS) ? (char *)0xdeadc0de : NULL;
    }
    crinitEnvSet_t e = {&envp[0], CRINIT_ENVSET_INITIAL_SIZE, CRINIT_ENVSET_SIZE_INCREMENT};

    expect_value_count(__wrap_free, ptr, (char *)0xdeadc0de, CRINIT_DUMMY_INITIALIZED_ELEMENTS);
    expect_value(__wrap_free, ptr, &envp[0]);

    assert_int_equal(crinitEnvSetDestroy(&e), 0);

    assert_ptr_equal(e.envp, NULL);
    assert_int_equal(e.allocSz, 0);
    assert_int_equal(e.allocInc, 0);
}
