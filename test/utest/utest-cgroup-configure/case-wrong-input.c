// SPDX-License-Identifier: MIT
/**
 * @file case-wrong-input.c
 * @brief Unit test for crinitConfConvToEnvSetMember(), handling of invalid string input.
 */

#include "cgroup.h"
#include "common.h"
#include "unit_test.h"

static void crinitExpectOpenAndCloseCgroupTest(char *validName) {
    int cgroupBaseFdTest = 42;
    int cgroupFdTest = 4711;

    expect_string(__wrap_open, pathname, CRINIT_CGROUP_PATH);
    expect_any(__wrap_open, flags);
    will_return(__wrap_open, cgroupBaseFdTest);

    expect_value(__wrap_mkdirat, dirfd, cgroupBaseFdTest);
    expect_value(__wrap_mkdirat, pathname, validName);
    expect_any(__wrap_mkdirat, mode);
    will_return(__wrap_mkdirat, 0);

    expect_value(__wrap_openat, dirfd, cgroupBaseFdTest);
    expect_string(__wrap_openat, pathname, validName);
    expect_any(__wrap_openat, flags);
    will_return(__wrap_openat, cgroupFdTest);

    expect_value(__wrap_close, fd, cgroupBaseFdTest);
    will_return(__wrap_close, 0);

    expect_value(__wrap_close, fd, cgroupFdTest);
    will_return(__wrap_close, 0);
}

void crinitCGroupConfigureTestWrongInput(void **state) {
    CRINIT_PARAM_UNUSED(state);

    /* valid Parameters*/
    crinitCgroupParam_t param1 = {"cgroup.freeze", "0"};
    crinitCgroupParam_t *validParam[] = {&param1};
    crinitCgroupConfiguration_t validConfig = {.param = validParam,
                                               .paramCount = sizeof(validParam) / sizeof(validParam[0])};
    char validName[] = "myCgroup";

    /* invalid Parameters*/
    crinitCgroupConfiguration_t *invalidConfig = NULL;
    crinitCgroupParam_t *invalidParam[] = {NULL};
    crinitCgroupConfiguration_t configWithInvalidParam1 = {.param = NULL, 2};
    crinitCgroupConfiguration_t configWithInvalidParam2 = {.param = invalidParam, 2};
    char *invalidName = NULL;

    crinitCgroup_t invalidNameCgroup = {0};
    invalidNameCgroup.name = invalidName;
    invalidNameCgroup.config = &validConfig;

    crinitCgroup_t invalidConfigCgroup = {0};
    invalidConfigCgroup.name = validName;
    invalidConfigCgroup.config = invalidConfig;

    crinitCgroup_t invalidParam1Cgroup = {0};
    invalidParam1Cgroup.name = validName;
    invalidParam1Cgroup.config = &configWithInvalidParam1;

    crinitCgroup_t invalidParam2Cgroup = {0};
    invalidParam2Cgroup.name = validName;
    invalidParam2Cgroup.config = &configWithInvalidParam2;

    assert_int_equal(crinitCGroupConfigure(NULL), -1);
    assert_int_equal(crinitCGroupConfigure(&invalidNameCgroup), -1);
    assert_int_equal(crinitCGroupConfigure(&invalidConfigCgroup), -1);
    assert_int_equal(crinitCGroupConfigure(&invalidParam1Cgroup), -1);

    crinitExpectOpenAndCloseCgroupTest(validName);
    assert_int_equal(crinitCGroupConfigure(&invalidParam2Cgroup), -1);
}
