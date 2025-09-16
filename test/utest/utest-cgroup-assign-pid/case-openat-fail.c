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

void crinitCgroupAssignPIDTestOpenatFailFirst(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitCgroup_t cgroupParent = {0};
    cgroupParent.name = "myGlobalCgroup";
    crinitCgroupParam_t *parentParam = NULL;
    crinitCgroupConfiguration_t parentConfig = {.param = parentParam, .paramCount = 0};
    cgroupParent.config = &parentConfig;

    crinitCgroup_t cgroup = {0};
    cgroup.name = "myCgroup";
    cgroup.parent = &cgroupParent;
    int cgroupBaseFdTest = 42;
    pid_t pidTest = 7815;

    expect_string(__wrap_open, pathname, CRINIT_CGROUP_PATH);
    expect_any(__wrap_open, flags);
    will_return(__wrap_open, cgroupBaseFdTest);

    expect_value(__wrap_openat, dirfd, cgroupBaseFdTest);
    expect_string(__wrap_openat, pathname, cgroup.parent->name);
    expect_any(__wrap_openat, flags);
    will_return(__wrap_openat, -1);

    expect_value(__wrap_close, fd, cgroupBaseFdTest);
    will_return(__wrap_close, 0);

    assert_int_equal(crinitCGroupAssignPID(&cgroup, pidTest), -1);
}

void crinitCgroupAssignPIDTestOpenatFailSecond(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitCgroup_t cgroup = {0};
    cgroup.name = "myCgroup";
    int cgroupBaseFdTest = 42;
    pid_t pidTest = 7815;

    expect_string(__wrap_open, pathname, CRINIT_CGROUP_PATH);
    expect_any(__wrap_open, flags);
    will_return(__wrap_open, cgroupBaseFdTest);

    expect_value(__wrap_openat, dirfd, cgroupBaseFdTest);
    expect_string(__wrap_openat, pathname, cgroup.name);
    expect_any(__wrap_openat, flags);
    will_return(__wrap_openat, -1);

    expect_value(__wrap_close, fd, cgroupBaseFdTest);
    will_return(__wrap_close, 0);

    assert_int_equal(crinitCGroupAssignPID(&cgroup, pidTest), -1);
}

void crinitCgroupAssignPIDTestOpenatFailThird(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitCgroup_t cgroup = {0};
    cgroup.name = "myCgroup";
    int cgroupBaseFdTest = 42;
    int cgroupFdTest = 4711;
    pid_t pidTest = 7815;

    expect_string(__wrap_open, pathname, CRINIT_CGROUP_PATH);
    expect_any(__wrap_open, flags);
    will_return(__wrap_open, cgroupBaseFdTest);

    expect_value(__wrap_openat, dirfd, cgroupBaseFdTest);
    expect_string(__wrap_openat, pathname, cgroup.name);
    expect_any(__wrap_openat, flags);
    will_return(__wrap_openat, cgroupFdTest);

    expect_value(__wrap_close, fd, cgroupBaseFdTest);
    will_return(__wrap_close, 0);

    expect_value(__wrap_openat, dirfd, cgroupFdTest);
    expect_string(__wrap_openat, pathname, "cgroup.procs");
    expect_any(__wrap_openat, flags);
    will_return(__wrap_openat, -1);

    expect_value(__wrap_close, fd, cgroupFdTest);
    will_return(__wrap_close, 0);

    assert_int_equal(crinitCGroupAssignPID(&cgroup, pidTest), -1);
}
