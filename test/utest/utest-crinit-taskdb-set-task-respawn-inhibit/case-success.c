// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitTaskDBSetTaskRespawnInhibit(), successful execution.
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

void crinitTaskDBSetTaskRespawnInhibitTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitConfKvList_t respawn = {.key = "RESPAWN", .val = "YES", .next = NULL};
    crinitConfKvList_t cmd = {.key = "COMMAND", .val = "/bin/true", .next = &respawn};
    crinitConfKvList_t name = {.key = "NAME", .val = "TEST", .next = &cmd};

    assert_int_equal(crinitGlobOptInitDefault(), 0);
    assert_int_equal(crinitTaskCreateFromConfKvList(&crinitTgt, &name), 0);
    assert_non_null(crinitTgt);
    assert_false(crinitTgt->inhibitRespawn);

    assert_int_equal(crinitTaskDBInitWithSize(&crinitCtx, crinitNullSpawnFunc, CRINIT_TASKDB_INITIAL_SIZE), 0);
    assert_int_equal(crinitTaskDBInsert(&crinitCtx, crinitTgt, true), 0);

    crinitTask_t *pTask = NULL;
    assert_int_equal(crinitTaskDBSetTaskRespawnInhibit(&crinitCtx, true, "TEST"), 0);
    assert_int_equal(crinitTaskDBGetTaskByName(&crinitCtx, &pTask, "TEST"), 0);
    assert_non_null(pTask);
    assert_true(pTask->inhibitRespawn);
    crinitDestroyTask(pTask);
    free(pTask);
}

int crinitTaskDBSetTaskRespawnInhibitTestSuccessTeardown(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitDestroyTask(crinitTgt);
    free(crinitTgt);
    crinitGlobOptDestroy();
    crinitTaskDBDestroy(&crinitCtx);

    return 0;
}
