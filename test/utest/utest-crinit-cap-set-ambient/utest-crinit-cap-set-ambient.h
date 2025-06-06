// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-cap-set-ambient.h
 * @brief Header declaring the unit tests for crinitCapSetAmbient().
 */
#ifndef __UTEST_CRINIT_CAP_SET_AMBIENT_H__
#define __UTEST_CRINIT_CAP_SET_AMBIENT_H__

void test_crinitCapSetAmbient_single_capability_set(void **state);
void test_crinitCapSetAmbient_multiple_capability_set(void **state);
void test_crinitCapSetAmbient_last_capability_set(void **state);
void test_crinitCapSetAmbient_no_capability_set(void **state);
void test_crinitCapSetAmbient_invalid_capability_typo(void **state);
void test_crinitCapSetAmbient_invalid_capability_range(void **state);

#endif /* __UTEST_CRINIT_CAP_SET_AMBIENT_H__ */
