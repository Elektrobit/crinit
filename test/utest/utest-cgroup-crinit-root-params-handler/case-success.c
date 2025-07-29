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
    assert_int_equal(crinitCfgCgroupRootParamsHandler(NULL, val, CRINIT_CONFIG_TYPE_SERIES), 0);
    crinitGlobOptStore_t *globOpts = crinitGlobOptBorrow();
    assert_non_null(globOpts);
    assert_non_null(globOpts->rootCgroup->config);
    assert_int_equal(globOpts->rootCgroup->config->paramCount, 1);
    assert_string_equal(globOpts->rootCgroup->config->param[0]->filename, "key");
    assert_string_equal(globOpts->rootCgroup->config->param[0]->option, "value");
    crinitGlobOptRemit();
    crinitGlobOptDestroy();
}

void crinitCfgCgroupRootParamsHandlerTestTwoKeyValueSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *val = "key=value \"anotherkey=another value\"";
    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitCfgCgroupRootParamsHandler(NULL, val, CRINIT_CONFIG_TYPE_SERIES), 0);
    crinitGlobOptStore_t *globOpts = crinitGlobOptBorrow();
    assert_non_null(globOpts);
    assert_non_null(globOpts->rootCgroup->config);
    assert_int_equal(globOpts->rootCgroup->config->paramCount, 2);
    assert_string_equal(globOpts->rootCgroup->config->param[0]->filename, "key");
    assert_string_equal(globOpts->rootCgroup->config->param[0]->option, "value");
    assert_string_equal(globOpts->rootCgroup->config->param[1]->filename, "anotherkey");
    assert_string_equal(globOpts->rootCgroup->config->param[1]->option, "another value");
    crinitGlobOptRemit();
    crinitGlobOptDestroy();
}
