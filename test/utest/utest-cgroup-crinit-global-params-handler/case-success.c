// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCfgCgroupGlobalParamsHandler(), successful execution.
 */

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "confhdl.h"
#include "globopt.h"
#include "unit_test.h"
#include "utest-cgroup-crinit-global-params-handler.h"

void crinitCfgCgroupGlobalParamsHandlerTestSingleKeyValueSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *cgname = "memory";
    const char *val = "memory:key=value";
    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitCfgCgroupGlobalNameHandler(NULL, cgname, CRINIT_CONFIG_TYPE_SERIES), 0);
    assert_int_equal(crinitCfgCgroupGlobalParamsHandler(NULL, val, CRINIT_CONFIG_TYPE_SERIES), 0);
    crinitGlobOptStore_t *globOpts = crinitGlobOptBorrow();
    assert_non_null(globOpts);
    assert_non_null(globOpts->globCgroups);
    assert_int_equal(globOpts->globCgroupsCount, 1);
    assert_non_null(globOpts->globCgroups[0]->config);
    assert_int_equal(globOpts->globCgroups[0]->config->paramCount, 1);
    assert_string_equal(globOpts->globCgroups[0]->config->param[0]->filename, "key");
    assert_string_equal(globOpts->globCgroups[0]->config->param[0]->option, "value");
    crinitGlobOptRemit();
    crinitGlobOptDestroy();
}

void crinitCfgCgroupGlobalParamsHandlerTestTwoKeyValueSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *cgname = "memory";
    const char *val = "memory:key=value \"memory:anotherkey=another value\"";
    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitCfgCgroupGlobalNameHandler(NULL, cgname, CRINIT_CONFIG_TYPE_SERIES), 0);
    assert_int_equal(crinitCfgCgroupGlobalParamsHandler(NULL, val, CRINIT_CONFIG_TYPE_SERIES), 0);
    crinitGlobOptStore_t *globOpts = crinitGlobOptBorrow();
    assert_non_null(globOpts);
    assert_non_null(globOpts->globCgroups);
    assert_int_equal(globOpts->globCgroupsCount, 1);
    assert_non_null(globOpts->globCgroups[0]->config);
    assert_int_equal(globOpts->globCgroups[0]->config->paramCount, 2);
    assert_string_equal(globOpts->globCgroups[0]->config->param[0]->filename, "key");
    assert_string_equal(globOpts->globCgroups[0]->config->param[0]->option, "value");
    assert_string_equal(globOpts->globCgroups[0]->config->param[1]->filename, "anotherkey");
    assert_string_equal(globOpts->globCgroups[0]->config->param[1]->option, "another value");
    crinitGlobOptRemit();
    crinitGlobOptDestroy();
}

void crinitCfgCgroupGlobalParamsHandlerTestTwoCGroupsOneKeyValueEachSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *cgname = "memory cpu";
    const char *val = "memory:key=value cpu:max=100";
    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitCfgCgroupGlobalNameHandler(NULL, cgname, CRINIT_CONFIG_TYPE_SERIES), 0);
    assert_int_equal(crinitCfgCgroupGlobalParamsHandler(NULL, val, CRINIT_CONFIG_TYPE_SERIES), 0);
    crinitGlobOptStore_t *globOpts = crinitGlobOptBorrow();
    assert_non_null(globOpts);
    assert_non_null(globOpts->globCgroups);
    assert_int_equal(globOpts->globCgroupsCount, 2);
    assert_non_null(globOpts->globCgroups[0]->config);
    assert_int_equal(globOpts->globCgroups[0]->config->paramCount, 1);
    assert_non_null(globOpts->globCgroups[1]->config);
    assert_int_equal(globOpts->globCgroups[1]->config->paramCount, 1);
    assert_string_equal(globOpts->globCgroups[0]->config->param[0]->filename, "key");
    assert_string_equal(globOpts->globCgroups[0]->config->param[0]->option, "value");
    assert_string_equal(globOpts->globCgroups[1]->config->param[0]->filename, "max");
    assert_string_equal(globOpts->globCgroups[1]->config->param[0]->option, "100");
    crinitGlobOptRemit();
    crinitGlobOptDestroy();
}
