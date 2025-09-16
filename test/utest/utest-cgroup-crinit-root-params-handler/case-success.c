// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCfgCgroupRootParamsHandler(), successful execution.
 */

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "confhdl.h"
#include "globopt.h"
#include "unit_test.h"
#include "utest-cgroup-crinit-root-params-handler.h"

void crinitCfgCgroupRootParamsHandlerTestSingleKeyValueSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *val = "key=value";
    assert_int_equal(crinitGlobOptInitDefault(), 0);
    crinitGlobOptStore_t *globOpts = crinitGlobOptBorrow();
    assert_non_null(globOpts);
    globOpts->rootCgroup = calloc(sizeof(*globOpts->rootCgroup), 1);
    assert_non_null(globOpts->rootCgroup);
    globOpts->rootCgroup->name = strdup("root.cg");
    assert_non_null(globOpts->rootCgroup->name);
    crinitGlobOptRemit();
    globOpts = NULL;
    assert_int_equal(crinitCfgCgroupRootParamsHandler(NULL, val, CRINIT_CONFIG_TYPE_SERIES), 0);
    globOpts = crinitGlobOptBorrow();
    assert_non_null(globOpts);
    assert_non_null(globOpts->rootCgroup->config);
    assert_int_equal(globOpts->rootCgroup->config->paramCount, 1);
    assert_string_equal(globOpts->rootCgroup->config->param[0].filename, "key");
    assert_string_equal(globOpts->rootCgroup->config->param[0].option, "value");
    crinitGlobOptRemit();
    crinitGlobOptDestroy();
}
