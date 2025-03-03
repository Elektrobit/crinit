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

void test_crinitTaskCreateFromConfKvListErrorInvalidSetCapabilityNames(void **state) {
    CRINIT_PARAM_UNUSED(state);

    static crinitTask_t *tgt = NULL;

    crinitConfKvList_t capClear = {.key = "CAPABILITY_CLEAR", .val = "CAP_CHOWN CAP_KILL"};
    crinitConfKvList_t capSet = {.key = "CAPABILITY_SET", .val = "CAP_KILL_TYPO", .next = &capClear};
    crinitConfKvList_t user = {.key = "USER", .val = "42", .next = &capSet};
    crinitConfKvList_t cmd = {.key = "COMMAND", .val = "/bin/true", .next = &user};
    crinitConfKvList_t name = {.key = "NAME", .val = "TESTCAP", .next = &cmd};

    will_return(__wrap_getpwuid_r, 0);

    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_not_equal(crinitTaskCreateFromConfKvList(&tgt, &name), 0);
    assert_false(tgt);
}

void test_crinitTaskCreateFromConfKvListErrorInvalidClearCapabilityNames(void **state) {
    CRINIT_PARAM_UNUSED(state);

    static crinitTask_t *tgt = NULL;

    crinitConfKvList_t capClear = {.key = "CAPABILITY_CLEAR", .val = "CAP_CHOWN CAP_KILL_TYPO"};
    crinitConfKvList_t capSet = {.key = "CAPABILITY_SET", .val = "CAP_KILL", .next = &capClear};
    crinitConfKvList_t user = {.key = "USER", .val = "42", .next = &capSet};
    crinitConfKvList_t cmd = {.key = "COMMAND", .val = "/bin/true", .next = &user};
    crinitConfKvList_t name = {.key = "NAME", .val = "TESTCAP", .next = &cmd};
    will_return(__wrap_getpwuid_r, 0);

    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_not_equal(crinitTaskCreateFromConfKvList(&tgt, &name), 0);
    assert_false(tgt);
}

int crinitTaskSetAndClearInvalidCapabilityNameTeardown(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitGlobOptDestroy();

    return 0;
}
