// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCfgUserHandler(), successful execution.
 */

#include <linux/capability.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "globopt.h"
#include "task.h"
#include "unit_test.h"
#include "utest-crinit-task-create-from-conf-kvlist.h"

static crinitTask_t *crinitTgt = NULL;

void test_crinitTaskCreateFromConfKvListErrorInvalidSetCapabilityDirective(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitConfKvList_t capClear = {.key = "CAPABILITY_CLEAR", .val = "CAP_CHOWN CAP_KILL"};
    crinitConfKvList_t capSet = {.key = "CAPABILITY_SET_TYPO", .val = "CAP_KILL", .next = &capClear};
    crinitConfKvList_t user = {.key = "USER", .val = "42", .next = &capSet};
    crinitConfKvList_t cmd = {.key = "COMMAND", .val = "/bin/true", .next = &user};
    crinitConfKvList_t name = {.key = "NAME", .val = "TESTCAP", .next = &cmd};
    will_return(__wrap_getpwuid_r, 0);

    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitTaskCreateFromConfKvList(&crinitTgt, &name), 0);
    assert_true(crinitTgt);
    assert_int_equal(crinitTgt->user, 42);
    assert_int_equal(crinitTgt->capabilitiesSet, 0);
    assert_int_equal(crinitTgt->capabilitiesClear, (1 << CAP_KILL) | (1 << CAP_CHOWN));
    assert_string_equal(crinitTgt->username, "www-run");
}

void test_crinitTaskCreateFromConfKvListErrorInvalidClearCapabilityDirective(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitConfKvList_t capClear = {.key = "CAPABILITY_CLEAR_TYPO", .val = "CAP_CHOWN CAP_KILL"};
    crinitConfKvList_t capSet = {.key = "CAPABILITY_SET", .val = "CAP_KILL", .next = &capClear};
    crinitConfKvList_t user = {.key = "USER", .val = "42", .next = &capSet};
    crinitConfKvList_t cmd = {.key = "COMMAND", .val = "/bin/true", .next = &user};
    crinitConfKvList_t name = {.key = "NAME", .val = "TESTCAP", .next = &cmd};
    will_return(__wrap_getpwuid_r, 0);

    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitTaskCreateFromConfKvList(&crinitTgt, &name), 0);
    assert_true(crinitTgt);
    assert_int_equal(crinitTgt->user, 42);
    assert_int_equal(crinitTgt->capabilitiesSet, 1 << CAP_KILL);
    assert_int_equal(crinitTgt->capabilitiesClear, 0);
    assert_string_equal(crinitTgt->username, "www-run");
}

int crinitTaskSetAndCleaInvalidCapabilityDirectiveTeardown(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitDestroyTask(crinitTgt);
    free(crinitTgt);
    crinitGlobOptDestroy();

    return 0;
}
