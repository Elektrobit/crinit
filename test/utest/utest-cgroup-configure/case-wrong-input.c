// SPDX-License-Identifier: MIT
/**
 * @file case-wrong-input.c
 * @brief Unit test for crinitConfConvToEnvSetMember(), handling of invalid string input.
 */

#include "cgroup.h"
#include "common.h"
#include "unit_test.h"

void crinitCGroupConfigureTestWrongInput(void **state) {
    CRINIT_PARAM_UNUSED(state);

    /* valid Parameters*/
    crinitCgroupParam_t validParam = {"cgroup.freeze", "0"};
    crinitCgroupConfiguration_t validConfig = {.param = &validParam, .paramCount = 1};
    char validName[] = "myCgroup";

    /* invalid Parameters*/
    crinitCgroupConfiguration_t *invalidConfig = NULL;
    crinitCgroupParam_t *invalidParam = NULL;
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

    assert_int_equal(crinitCGroupConfigure(&invalidParam2Cgroup), -1);
}
