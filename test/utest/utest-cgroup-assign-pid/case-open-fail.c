// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitFileSeriesFromDir(), successful execution.
 */

#include <stdio.h>
#include <sys/types.h>

#include "cgroup.h"
#include "common.h"
#include "unit_test.h"

void crinitCgroupAssignPIDTestOpenFail(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitCgroup_t cgroup = {0};
    cgroup.name = "myCgroup";
    pid_t pidTest = 7815;

    expect_string(__wrap_open, pathname, CRINIT_CGROUP_PATH);
    expect_any(__wrap_open, flags);
    will_return(__wrap_open, -1);

    assert_int_equal(crinitCGroupAssignPID(&cgroup, pidTest), -1);
}
