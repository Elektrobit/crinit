// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitFileSeriesFromDir(), successful execution.
 */
#ifdef ENABLE_CGROUP
#include <stdio.h>
#include <sys/types.h>

#include "cgroup.h"
#include "common.h"
#include "unit_test.h"

void crinitCGroupConfigureTestOpenFail(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitCgroup_t cgroup = {0};
    cgroup.name = "myCgroup";

    crinitCgroupParam_t param1 = {"memory.max", "536870912"};
    crinitCgroupParam_t param2 = {"pids.max", "100"};

    crinitCgroupParam_t param[] = {param1, param2};

    crinitCgroupConfiguration_t config = {.param = param, .paramCount = sizeof(param) / sizeof(param[0])};
    cgroup.config = &config;

    expect_string(__wrap_open, pathname, CRINIT_CGROUP_PATH);
    expect_any(__wrap_open, flags);
    will_return(__wrap_open, -1);

    assert_int_equal(crinitCGroupConfigure(&cgroup), -1);
}
#endif
