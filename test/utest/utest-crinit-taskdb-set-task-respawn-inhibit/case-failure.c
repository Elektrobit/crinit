// SPDX-License-Identifier: MIT
/**
 * @file case-failure.c
 * @brief Unit test for crinitTaskDBSetTaskRespawnInhibit(), failure execution.
 */

#include <stdlib.h>
#include <string.h>
#include <sys/capability.h>

#include "common.h"
#include "globopt.h"
#include "task.h"
#include "taskdb.h"
#include "unit_test.h"
#include "utest-crinit-taskdb-set-task-respawn-inhibit.h"

static crinitTask_t *crinitTgt = NULL;
static crinitTaskDB_t crinitCtx;

static int crinitNullSpawnFunc(crinitTaskDB_t *ctx, const crinitTask_t *t, crinitDispatchThreadMode_t mode) {
    CRINIT_PARAM_UNUSED(ctx);
    CRINIT_PARAM_UNUSED(t);
    CRINIT_PARAM_UNUSED(mode);

    return 0;
}

void crinitTaskDBSetTaskRespawnInhibitTestCtxNullPointerFailure(void **state) {
    CRINIT_PARAM_UNUSED(state);

    assert_int_equal(crinitTaskDBSetTaskRespawnInhibit(NULL, true, "TEST"), -1);
}

void crinitTaskDBSetTaskRespawnInhibitTestTaskNameNullPointerFailure(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitConfKvList_t respawn = {.key = "RESPAWN", .val = "YES", .next = NULL};
    crinitConfKvList_t cmd = {.key = "COMMAND", .val = "/bin/true", .next = &respawn};
    crinitConfKvList_t name = {.key = "NAME", .val = "TEST", .next = &cmd};

    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitTaskCreateFromConfKvList(&crinitTgt, &name), 0);
    assert_non_null(crinitTgt);

    assert_int_equal(crinitTaskDBInitWithSize(&crinitCtx, crinitNullSpawnFunc, CRINIT_TASKDB_INITIAL_SIZE), 0);
    assert_int_equal(crinitTaskDBInsert(&crinitCtx, crinitTgt, true), 0);

    assert_int_equal(crinitTaskDBSetTaskRespawnInhibit(&crinitCtx, true, NULL), -1);
}

void crinitTaskDBSetTaskRespawnInhibitTestTaskNotFoundFailure(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitConfKvList_t respawn = {.key = "RESPAWN", .val = "YES", .next = NULL};
    crinitConfKvList_t cmd = {.key = "COMMAND", .val = "/bin/true", .next = &respawn};
    crinitConfKvList_t name = {.key = "NAME", .val = "TEST", .next = &cmd};

    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitTaskCreateFromConfKvList(&crinitTgt, &name), 0);
    assert_non_null(crinitTgt);

    assert_int_equal(crinitTaskDBInitWithSize(&crinitCtx, crinitNullSpawnFunc, CRINIT_TASKDB_INITIAL_SIZE), 0);
    assert_int_equal(crinitTaskDBInsert(&crinitCtx, crinitTgt, true), 0);

    assert_int_equal(crinitTaskDBSetTaskRespawnInhibit(&crinitCtx, true, "fooBar"), -1);
}

int crinitTaskDBSetTaskRespawnInhibitTestFailureTeardown(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitDestroyTask(crinitTgt);
    free(crinitTgt);
    crinitGlobOptDestroy();
    crinitTaskDBDestroy(&crinitCtx);

    return 0;
}
