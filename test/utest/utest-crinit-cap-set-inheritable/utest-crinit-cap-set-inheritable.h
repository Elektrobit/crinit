// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-cap-set-inheritable.h
 * @brief Header declaring the unit tests for crinitCapSetInheritable().
 */
#ifndef __UTEST_CRINIT_CAP_SET_INHERITABLE_H__
#define __UTEST_CRINIT_CAP_SET_INHERITABLE_H__

void test_crinitCapSetInheritable(void **state);
void test_crinitCapSetInheritable_last_supported(void **state);
void test_crinitCapSetInheritable_invalid_capability(void **state);
void test_crinitCapSetInheritable_invalid_capability_range(void **state);

#endif /* __UTEST_CRINIT_CAP_SET_INHERITABLE_H__ */
