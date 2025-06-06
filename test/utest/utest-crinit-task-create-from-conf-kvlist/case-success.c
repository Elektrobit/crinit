// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCfgUserHandler(), successful execution.
 */

#include <stdlib.h>
#include <string.h>
#include <sys/capability.h>

#include "common.h"
#include "globopt.h"
#include "task.h"
#include "unit_test.h"
#include "utest-crinit-task-create-from-conf-kvlist.h"

static crinitTask_t *crinitTgt = NULL;

void crinitTaskCreateFromConfKvListTestGroupNumericSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitConfKvList_t group = {.key = "GROUP", .val = "42"};
    crinitConfKvList_t cmd = {.key = "COMMAND", .val = "/bin/true", .next = &group};
    crinitConfKvList_t name = {.key = "NAME", .val = "TEST", .next = &cmd};

    will_return(__wrap_getgrgid_r, 0);

    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitTaskCreateFromConfKvList(&crinitTgt, &name), 0);
    assert_true(crinitTgt);
    assert_int_equal(crinitTgt->group, 42);
    assert_string_equal(crinitTgt->groupname, "disk");
}

void crinitTaskCreateFromConfKvListTestUserNumericSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitConfKvList_t user = {.key = "USER", .val = "42"};
    crinitConfKvList_t cmd = {.key = "COMMAND", .val = "/bin/true", .next = &user};
    crinitConfKvList_t name = {.key = "NAME", .val = "TEST", .next = &cmd};

    will_return(__wrap_getpwuid_r, 0);

    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitTaskCreateFromConfKvList(&crinitTgt, &name), 0);
    assert_true(crinitTgt);
    assert_int_equal(crinitTgt->user, 42);
    assert_string_equal(crinitTgt->username, "www-run");
}

#ifdef ENABLE_CAPABILITIES
void test_crinitTaskCreateFromConfKvListSuccessSetAnClearCaps(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitConfKvList_t capClear = {.key = "CAPABILITY_CLEAR", .val = "CAP_CHOWN"};
    crinitConfKvList_t capSet = {.key = "CAPABILITY_SET", .val = "CAP_KILL", .next = &capClear};
    crinitConfKvList_t user = {.key = "USER", .val = "42", .next = &capSet};
    crinitConfKvList_t cmd = {.key = "COMMAND", .val = "/bin/true", .next = &user};
    crinitConfKvList_t name = {.key = "NAME", .val = "TESTCAP", .next = &cmd};

    will_return(__wrap_getpwuid_r, 0);

    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitTaskCreateFromConfKvList(&crinitTgt, &name), 0);
    assert_true(crinitTgt);
    assert_int_equal(crinitTgt->user, 42);
    assert_string_equal(crinitTgt->username, "www-run");

    assert_int_equal(crinitTgt->capabilitiesSet, 1 << CAP_KILL);

    assert_int_equal(crinitTgt->capabilitiesClear, 1 << CAP_CHOWN);
}

void test_crinitTaskCreateFromConfKvListSuccessSetAnClearMultipleCaps(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitConfKvList_t capClear = {.key = "CAPABILITY_CLEAR", .val = "CAP_CHOWN CAP_KILL"};
    crinitConfKvList_t capSet = {.key = "CAPABILITY_SET", .val = "CAP_KILL CAP_DAC_OVERRIDE", .next = &capClear};
    crinitConfKvList_t user = {.key = "USER", .val = "42", .next = &capSet};
    crinitConfKvList_t cmd = {.key = "COMMAND", .val = "/bin/true", .next = &user};
    crinitConfKvList_t name = {.key = "NAME", .val = "TESTCAP", .next = &cmd};

    will_return(__wrap_getpwuid_r, 0);

    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitTaskCreateFromConfKvList(&crinitTgt, &name), 0);
    assert_true(crinitTgt);
    assert_int_equal(crinitTgt->user, 42);
    assert_string_equal(crinitTgt->username, "www-run");

    assert_int_equal(crinitTgt->capabilitiesSet, (1 << CAP_KILL | 1 << CAP_DAC_OVERRIDE));

    assert_int_equal(crinitTgt->capabilitiesClear, (1 << CAP_KILL | 1 << CAP_CHOWN));
}
#endif

int crinitTaskCreateFromConfKvListTestTeardown(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitDestroyTask(crinitTgt);
    free(crinitTgt);
    crinitGlobOptDestroy();

    return 0;
}
