// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-cap-get-inheritable.c
 * @brief Header declaring the unit tests for crinitCapGetInheritable().
 */
#ifndef __UTEST_CRINIT_CAP_GET_INHERITABLE_H__
#define __UTEST_CRINIT_CAP_GET_INHERITABLE_H__

void test_crinitCapGetInheritable(void **state);
void test_crinitCapGetInheritable_resultParmInitialized(void **state);
void test_crinitCapGetInheritable_invalid_capability_pointer(void **state);

#endif /* __UTEST_CRINIT_CAP_GET_INHERITABLE_H__ */
