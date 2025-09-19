// SPDX-License-Identifier: MIT
/**
 * @file case-null-input.c
 * @brief Unit test for crinitCfgCgroupRootNameHandler(), input parameter is empty.
 */
#ifdef ENABLE_CGROUP
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "confhdl.h"
#include "globopt.h"
#include "unit_test.h"
#include "utest-cgroup-crinit-root-name-handler.h"

void crinitCfgCgroupRootNameHandlerTestEmptyInput(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *val = "";
    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitCfgCgroupRootNameHandler(NULL, val, CRINIT_CONFIG_TYPE_SERIES), -1);
    crinitGlobOptStore_t *globOpts = crinitGlobOptBorrow();
    assert_non_null(globOpts);
    assert_null(globOpts->rootCgroup);
    crinitGlobOptRemit();
    crinitGlobOptDestroy();
}
#endif
