// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-task-create-from-conf-kvlist.h
 * @brief Header declaring the unit tests for crinitTaskCreateFromConfKvList().
 */
#ifndef __UTEST_TASK_CREATE_FROM_CONF_KVLIST_H__
#define __UTEST_TASK_CREATE_FROM_CONF_KVLIST_H__

/**
 * Tests successful parsing key "GROUP" with numeric value.
 */
void crinitTaskCreateFromConfKvListTestGroupNumericSuccess(void **state);
/**
 * Tests successful parsing key "USER" with numeric value.
 */
void crinitTaskCreateFromConfKvListTestUserNumericSuccess(void **state);
int crinitTaskCreateFromConfKvListTestTeardown(void **state);

#ifdef ENABLE_CAPABILITIES
void test_crinitTaskCreateFromConfKvListSuccessSetAnClearCaps(void **state);
void test_crinitTaskCreateFromConfKvListSuccessSetAnClearMultipleCaps(void **state);
int crinitTaskSetAndClearCapabilitiesTeardown(void **state);

void test_crinitTaskCreateFromConfKvListErrorInvalidSetCapabilityNames(void **state);
void test_crinitTaskCreateFromConfKvListErrorInvalidClearCapabilityNames(void **state);
int crinitTaskSetAndClearInvalidCapabilityNameTeardown(void **state);

void test_crinitTaskCreateFromConfKvListErrorInvalidSetCapabilityDirective(void **state);
void test_crinitTaskCreateFromConfKvListErrorInvalidClearCapabilityDirective(void **state);
int crinitTaskSetAndCleaInvalidCapabilityDirectiveTeardown(void **state);
#endif

#endif /* __UTEST_TASK_CREATE_FROM_CONF_KVLIST_H__ */
