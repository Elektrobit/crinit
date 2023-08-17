// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitEnvSetDup(), successful execution.
 */

#include "common.h"
#include "envset.h"
#include "string.h"
#include "unit_test.h"
#include "utest-envset-dup.h"

#define UTEST_ENVSET_DUP_ORIG_SET_ALLOCSIZE 8uL
#define UTEST_ENVSET_DUP_ORIG_SET_ELEMENTS (UTEST_ENVSET_DUP_ORIG_SET_ALLOCSIZE - 3uL)

void crinitEnvSetDupTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    char dummyStr[] = "foo bar baz";
    char *origEnvp[UTEST_ENVSET_DUP_ORIG_SET_ALLOCSIZE];
    for (size_t i = 0; i < UTEST_ENVSET_DUP_ORIG_SET_ALLOCSIZE; i++) {
        origEnvp[i] = (i < UTEST_ENVSET_DUP_ORIG_SET_ELEMENTS) ? dummyStr : NULL;
    }
    crinitEnvSet_t origSet = {&origEnvp[0], sizeof(origEnvp), sizeof(origEnvp) / 2};
    crinitEnvSet_t copySet = {NULL, 0, 0};

    assert_int_equal(crinitEnvSetDup(&copySet, &origSet), 0);

    assert_non_null(copySet.envp);
    assert_int_equal(copySet.allocSz, origSet.allocSz);
    assert_int_equal(copySet.allocInc, origSet.allocInc);
    for (size_t i = 0; i < UTEST_ENVSET_DUP_ORIG_SET_ALLOCSIZE; i++) {
        if (i < UTEST_ENVSET_DUP_ORIG_SET_ELEMENTS) {
            assert_string_equal(copySet.envp[i], dummyStr);
        } else {
            assert_null(copySet.envp[i]);
        }
    }
    assert_int_equal(crinitEnvSetDestroy(&copySet), 0);
}
