// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-task-create-from-conf-kvlist.h
 * @brief Header declaring the unit tests for crinitTaskCreateFromConfKvList().
 */
#ifndef __UTEST_TASK_CREATE_FROM_CONF_KVLIST_H__
#define __UTEST_TASK_CREATE_FROM_CONF_KVLIST_H__

/**
 * Tests successful parsing key "GROUP".
 */
void crinitTaskCreateFromConfKvListTestGroupSuccess(void **state);
/**
 * Tests successful parsing key "USER".
 */
void crinitTaskCreateFromConfKvListTestUserSuccess(void **state);

#endif /* __UTEST_TASK_CREATE_FROM_CONF_KVLIST_H__ */
