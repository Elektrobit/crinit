// SPDX-License-Identifier: MIT
/**
 * @file case-null-input.c
 * @brief Unit test for crinitCfgCgroupRootParamsHandler(), input parameter is NULL.
 */

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "confhdl.h"
#include "globopt.h"
#include "unit_test.h"
#include "utest-cgroup-crinit-root-params-handler.h"

void crinitCfgCgroupRootParamsHandlerTestNullInput(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *val = NULL;
    assert_int_equal(crinitGlobOptInitDefault(), 0);
    crinitGlobOptStore_t *globOpts = crinitGlobOptBorrow();
    assert_non_null(globOpts);
    globOpts->rootCgroup = calloc(sizeof(*globOpts->rootCgroup), 1);
    assert_non_null(globOpts->rootCgroup);
    globOpts->rootCgroup->name = strdup("root.cg");
    assert_non_null(globOpts->rootCgroup->name);
    crinitGlobOptRemit();
    globOpts = NULL;
    assert_int_equal(crinitCfgCgroupRootParamsHandler(NULL, val, CRINIT_CONFIG_TYPE_SERIES), -1);
    globOpts = crinitGlobOptBorrow();
    assert_non_null(globOpts);
    assert_null(globOpts->rootCgroup->config);
    crinitGlobOptRemit();
    crinitGlobOptDestroy();
}
