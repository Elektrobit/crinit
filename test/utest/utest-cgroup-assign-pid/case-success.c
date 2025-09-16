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

void crinitCgroupAssignPIDTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitCgroup_t cgroup = {0};
    cgroup.name = "myCgroup";
    int cgroupBaseFdTest = 42;
    int cgroupFdTest = 4711;
    int cgroupOptionFdTest = 0x0815;
    pid_t pidTest = 7815;
    /*length of pidTest (=4) plus EOL (=1)*/
    int written = 5;

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
    will_return(__wrap_openat, cgroupOptionFdTest);

    expect_value(__wrap_writev, fd, cgroupOptionFdTest);
    expect_any(__wrap_writev, iov);
    expect_any(__wrap_writev, iovcnt);
    will_return(__wrap_writev, written);

    expect_value(__wrap_close, fd, cgroupOptionFdTest);
    will_return(__wrap_close, 0);

    expect_value(__wrap_close, fd, cgroupFdTest);
    will_return(__wrap_close, 0);

    assert_int_equal(crinitCGroupAssignPID(&cgroup, pidTest), 0);
}

void crinitCgroupAssignPIDTestSuccessParent(void **state) {
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
    int cgroupParentFdTest = 123;
    int cgroupFdTest = 4711;
    int cgroupOptionFdTest = 0x0815;
    pid_t pidTest = 7815;
    /*length of pidTest (=4) plus EOL (=1)*/
    int written = 5;

    expect_string(__wrap_open, pathname, CRINIT_CGROUP_PATH);
    expect_any(__wrap_open, flags);
    will_return(__wrap_open, cgroupBaseFdTest);

    expect_value(__wrap_openat, dirfd, cgroupBaseFdTest);
    expect_string(__wrap_openat, pathname, cgroupParent.name);
    expect_any(__wrap_openat, flags);
    will_return(__wrap_openat, cgroupParentFdTest);

    expect_value(__wrap_openat, dirfd, cgroupParentFdTest);
    expect_string(__wrap_openat, pathname, cgroup.name);
    expect_any(__wrap_openat, flags);
    will_return(__wrap_openat, cgroupFdTest);

    expect_value(__wrap_close, fd, cgroupBaseFdTest);
    will_return(__wrap_close, 0);

    expect_value(__wrap_close, fd, cgroupParentFdTest);
    will_return(__wrap_close, 0);

    expect_value(__wrap_openat, dirfd, cgroupFdTest);
    expect_string(__wrap_openat, pathname, "cgroup.procs");
    expect_any(__wrap_openat, flags);
    will_return(__wrap_openat, cgroupOptionFdTest);

    expect_value(__wrap_writev, fd, cgroupOptionFdTest);
    expect_any(__wrap_writev, iov);
    expect_any(__wrap_writev, iovcnt);
    will_return(__wrap_writev, written);

    expect_value(__wrap_close, fd, cgroupOptionFdTest);
    will_return(__wrap_close, 0);

    expect_value(__wrap_close, fd, cgroupFdTest);
    will_return(__wrap_close, 0);

    assert_int_equal(crinitCGroupAssignPID(&cgroup, pidTest), 0);
}
#endif
