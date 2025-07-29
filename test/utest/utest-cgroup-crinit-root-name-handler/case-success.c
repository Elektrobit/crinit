// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCfgCgroupRootNameHandler(), successful execution.
 */

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "confhdl.h"
#include "globopt.h"
#include "unit_test.h"
#include "utest-cgroup-crinit-root-name-handler.h"

void crinitCfgCgroupRootNameHandlerTestAlphaInputSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *val = "test.cg";
    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitCfgCgroupRootNameHandler(NULL, val, CRINIT_CONFIG_TYPE_SERIES), 0);
    crinitGlobOptStore_t *globOpts = crinitGlobOptBorrow();
    assert_non_null(globOpts);
    assert_string_equal(globOpts->rootCgroup->name, "test.cg");
    crinitGlobOptRemit();
    crinitGlobOptDestroy();
}
