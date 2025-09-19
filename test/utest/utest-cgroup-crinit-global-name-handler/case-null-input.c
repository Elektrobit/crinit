// SPDX-License-Identifier: MIT
/**
 * @file case-null-input.c
 * @brief Unit test for crinitCfgCgroupGlobalNameHandler(), input parameter is NULL.
 */
#ifdef ENABLE_CGROUP
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "confhdl.h"
#include "globopt.h"
#include "unit_test.h"
#include "utest-cgroup-crinit-global-name-handler.h"

void crinitCfgCgroupGlobalNameHandlerTestNullInput(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *val = NULL;
    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitCfgCgroupGlobalNameHandler(NULL, val, CRINIT_CONFIG_TYPE_SERIES), -1);
    crinitGlobOptStore_t *globOpts = crinitGlobOptBorrow();
    assert_non_null(globOpts);
    assert_int_equal(globOpts->globCgroupsCount, 0);
    assert_null(globOpts->globCgroups);
    crinitGlobOptRemit();
    crinitGlobOptDestroy();
}
#endif
