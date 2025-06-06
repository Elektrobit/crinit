// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-cap-is-capset-effective.h
 * @brief Header declaring the unit tests for test_crinitCapIsCapsetEffective().
 */
#ifndef __UTEST_CRINIT_CAP_IS_CAPSET_EFFECTIVE_H__
#define __UTEST_CRINIT_CAP_IS_CAPSET_EFFECTIVE_H__

void test_crinitCapIsCapsetEffective_not_set(void **state);
void test_crinitCapIsCapsetEffective_lsb_low(void **state);
void test_crinitCapIsCapsetEffective_msb_low(void **state);
void test_crinitCapIsCapsetEffective_lsb_high(void **state);
void test_crinitCapIsCapsetEffective_not_set(void **state);
void test_crinitCapIsCapsetEffective_last_supported(void **state);
void test_crinitCapIsCapsetEffective_unsupported_capability(void **state);
void test_crinitCapIsCapsetEffective_first_after_last_capability(void **state);
void test_crinitCapIsCapsetEffective_last_possible_capability(void **state);

#endif /* __UTEST_CRINIT_CAP_IS_CAPSET_EFFECTIVE_H__ */
