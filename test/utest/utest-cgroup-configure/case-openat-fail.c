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

void crinitCGroupConfigureTestOpenatFailFirst(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitCgroup_t cgroupParent = {0};
    cgroupParent.name = "myGlobalCgroup";
    crinitCgroupParam_t *parentParam = NULL;
    crinitCgroupConfiguration_t parentConfig = {.param = parentParam, .paramCount = 0};
    cgroupParent.config = &parentConfig;

    crinitCgroup_t cgroup = {0};
    cgroup.name = "myCgroup";
    cgroup.parent = &cgroupParent;

    crinitCgroupParam_t param1 = {"memory.max", "536870912"};
    crinitCgroupParam_t param2 = {"pids.max", "100"};

    crinitCgroupParam_t param[] = {param1, param2};

    crinitCgroupConfiguration_t config = {.param = param, .paramCount = sizeof(param) / sizeof(param[0])};
    cgroup.config = &config;

    int cgroupBaseFdTest = 42;

    expect_string(__wrap_open, pathname, CRINIT_CGROUP_PATH);
    expect_any(__wrap_open, flags);
    will_return(__wrap_open, cgroupBaseFdTest);

    expect_value(__wrap_openat, dirfd, cgroupBaseFdTest);
    expect_string(__wrap_openat, pathname, cgroup.parent->name);
    expect_any(__wrap_openat, flags);
    will_return(__wrap_openat, -1);

    expect_value(__wrap_close, fd, cgroupBaseFdTest);
    will_return(__wrap_close, 0);

    assert_int_equal(crinitCGroupConfigure(&cgroup), -1);
}

void crinitCGroupConfigureTestOpenatFailSecond(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitCgroup_t cgroup = {0};
    cgroup.name = "myCgroup";

    crinitCgroupParam_t param1 = {"memory.max", "536870912"};
    crinitCgroupParam_t param2 = {"pids.max", "100"};

    crinitCgroupParam_t param[] = {param1, param2};

    crinitCgroupConfiguration_t config = {.param = param, .paramCount = sizeof(param) / sizeof(param[0])};
    cgroup.config = &config;

    int cgroupBaseFdTest = 42;

    expect_string(__wrap_open, pathname, CRINIT_CGROUP_PATH);
    expect_any(__wrap_open, flags);
    will_return(__wrap_open, cgroupBaseFdTest);

    expect_value(__wrap_mkdirat, dirfd, cgroupBaseFdTest);
    expect_value(__wrap_mkdirat, pathname, cgroup.name);
    expect_any(__wrap_mkdirat, mode);
    will_return(__wrap_mkdirat, 0);

    expect_value(__wrap_openat, dirfd, cgroupBaseFdTest);
    expect_string(__wrap_openat, pathname, cgroup.name);
    expect_any(__wrap_openat, flags);
    will_return(__wrap_openat, -1);

    expect_value(__wrap_close, fd, cgroupBaseFdTest);
    will_return(__wrap_close, 0);

    assert_int_equal(crinitCGroupConfigure(&cgroup), -1);
}

void crinitCGroupConfigureTestOpenatFailThird(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitCgroup_t cgroup = {0};
    cgroup.name = "myCgroup";

    crinitCgroupParam_t param1 = {"memory.max", "536870912"};
    crinitCgroupParam_t param2 = {"pids.max", "100"};

    crinitCgroupParam_t param[] = {param1, param2};

    crinitCgroupConfiguration_t config = {.param = param, .paramCount = sizeof(param) / sizeof(param[0])};
    cgroup.config = &config;

    int cgroupBaseFdTest = 42;
    int cgroupFdTest = 4711;

    expect_string(__wrap_open, pathname, CRINIT_CGROUP_PATH);
    expect_any(__wrap_open, flags);
    will_return(__wrap_open, cgroupBaseFdTest);

    expect_value(__wrap_mkdirat, dirfd, cgroupBaseFdTest);
    expect_value(__wrap_mkdirat, pathname, cgroup.name);
    expect_any(__wrap_mkdirat, mode);
    will_return(__wrap_mkdirat, 0);

    expect_value(__wrap_openat, dirfd, cgroupBaseFdTest);
    expect_string(__wrap_openat, pathname, cgroup.name);
    expect_any(__wrap_openat, flags);
    will_return(__wrap_openat, cgroupFdTest);

    expect_value(__wrap_close, fd, cgroupBaseFdTest);
    will_return(__wrap_close, 0);

    expect_value(__wrap_openat, dirfd, cgroupFdTest);
    expect_string(__wrap_openat, pathname, param1.filename);
    expect_any(__wrap_openat, flags);
    will_return(__wrap_openat, -1);

    expect_value(__wrap_close, fd, cgroupFdTest);
    will_return(__wrap_close, 0);

    assert_int_equal(crinitCGroupConfigure(&cgroup), -1);
}
#endif
