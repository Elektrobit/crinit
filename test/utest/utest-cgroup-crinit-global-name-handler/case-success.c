// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCfgCgroupGlobalNameHandler(), successful execution.
 */
#ifdef ENABLE_CGROUP
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "confhdl.h"
#include "globopt.h"
#include "unit_test.h"
#include "utest-cgroup-crinit-global-name-handler.h"

void crinitCfgCgroupGlobalNameHandlerTestAlphaInputOneValueSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *val = "test.cg";
    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitCfgCgroupGlobalNameHandler(NULL, val, CRINIT_CONFIG_TYPE_SERIES), 0);
    crinitGlobOptStore_t *globOpts = crinitGlobOptBorrow();
    assert_non_null(globOpts);
    assert_int_equal(globOpts->globCgroupsCount, 1);
    assert_string_equal(globOpts->globCgroups[0]->name, "test.cg");
    crinitGlobOptRemit();
    crinitGlobOptDestroy();
}

void crinitCfgCgroupGlobalNameHandlerTestAlphaInputTwoValuesSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *val = "test.cg test2.cg";
    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitCfgCgroupGlobalNameHandler(NULL, val, CRINIT_CONFIG_TYPE_SERIES), 0);
    crinitGlobOptStore_t *globOpts = crinitGlobOptBorrow();
    assert_non_null(globOpts);
    assert_int_equal(globOpts->globCgroupsCount, 2);
    assert_string_equal(globOpts->globCgroups[0]->name, "test.cg");
    assert_string_equal(globOpts->globCgroups[1]->name, "test2.cg");
    crinitGlobOptRemit();
    crinitGlobOptDestroy();
}
#endif
