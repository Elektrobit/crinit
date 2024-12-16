// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCfgUserHandler(), successful execution.
 */

#include "task.h"
#include "common.h"
#include "globopt.h"
#include "unit_test.h"
#include "utest-crinit-task-create-from-conf-kvlist.h"

#include <stdlib.h>
#include <string.h>

void crinitTaskCreateFromConfKvListTestGroupNumericSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTask_t *tgt = NULL;
    crinitConfKvList_t name;
    crinitConfKvList_t cmd;
    crinitConfKvList_t group;
    memset(&name, 0x00, sizeof(name));
    memset(&cmd, 0x00, sizeof(cmd));
    memset(&group, 0x00, sizeof(group));
    name.key = "NAME";
    name.val = "TEST";
    name.next = &cmd;
    cmd.key = "COMMAND";
    cmd.val = "/bin/true";
    cmd.next = &group;
    group.key = "GROUP";
    group.val = "42";

    will_return(__wrap_getgrgid_r, 0);

    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitTaskCreateFromConfKvList(&tgt, &name), 0);
    assert_true(tgt);
    assert_int_equal(tgt->group, 42);
    assert_string_equal(tgt->groupname, "disk");
    free(tgt->username);
    free(tgt->groupname);
    free(tgt);
    crinitGlobOptDestroy();
}

void crinitTaskCreateFromConfKvListTestUserNumericSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTask_t *tgt = NULL;
    crinitConfKvList_t name;
    crinitConfKvList_t cmd;
    crinitConfKvList_t user;
    memset(&name, 0x00, sizeof(name));
    memset(&cmd, 0x00, sizeof(cmd));
    memset(&user, 0x00, sizeof(user));
    name.key = "NAME";
    name.val = "TEST";
    name.next = &cmd;
    cmd.key = "COMMAND";
    cmd.val = "/bin/true";
    cmd.next = &user;
    user.key = "USER";
    user.val = "42";

    will_return(__wrap_getpwuid_r, 0);

    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitTaskCreateFromConfKvList(&tgt, &name), 0);
    assert_true(tgt);
    assert_int_equal(tgt->user, 42);
    assert_string_equal(tgt->username, "www-run");
    free(tgt->username);
    free(tgt->groupname);
    free(tgt);
    crinitGlobOptDestroy();
}
