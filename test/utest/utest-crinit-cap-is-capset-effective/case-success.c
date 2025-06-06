// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCapIsCapsetEffective(), successful execution.
 */

#include <sys/syscall.h>

#include "capabilities.h"
#include "common.h"
#include "unit_test.h"
#include "utest-crinit-cap-is-capset-effective.h"

#define INH_CAP_MASK_DONT_CARE 0
#define PID_DONT_CARE 0

void test_crinitCapIsCapsetEffective_lsb_low(void **state) {
    CRINIT_PARAM_UNUSED(state);
    expect_not_value(__wrap_syscall, hdr, NULL);
    expect_value(__wrap_syscall, hdr->version, _LINUX_CAPABILITY_VERSION_3);
    expect_value(__wrap_syscall, hdr->pid, PID_DONT_CARE);
    expect_not_value(__wrap_syscall, out, NULL);
    expect_value(__wrap_syscall, number, SYS_capget);
    will_return_count(__wrap_syscall, INH_CAP_MASK_DONT_CARE, 2);
    will_return(__wrap_syscall, 1 << CAP_CHOWN);
    will_return(__wrap_syscall, 0);
    assert_int_equal(crinitCapIsCapsetEffective(CAP_CHOWN, PID_DONT_CARE), true);
}

void test_crinitCapIsCapsetEffective_msb_low(void **state) {
    CRINIT_PARAM_UNUSED(state);

    expect_not_value(__wrap_syscall, hdr, NULL);
    expect_value(__wrap_syscall, hdr->version, _LINUX_CAPABILITY_VERSION_3);
    expect_value(__wrap_syscall, hdr->pid, PID_DONT_CARE);
    expect_not_value(__wrap_syscall, out, NULL);
    expect_value(__wrap_syscall, number, SYS_capget);
    will_return_count(__wrap_syscall, INH_CAP_MASK_DONT_CARE, 2);
    will_return(__wrap_syscall, 1 << CAP_SETFCAP);
    will_return(__wrap_syscall, 0);
    assert_int_equal(crinitCapIsCapsetEffective(CAP_SETFCAP, PID_DONT_CARE), true);
}

void test_crinitCapIsCapsetEffective_lsb_high(void **state) {
    CRINIT_PARAM_UNUSED(state);

    expect_not_value(__wrap_syscall, hdr, NULL);
    expect_value(__wrap_syscall, hdr->version, _LINUX_CAPABILITY_VERSION_3);
    expect_value(__wrap_syscall, hdr->pid, PID_DONT_CARE);
    expect_not_value(__wrap_syscall, out, NULL);
    expect_value(__wrap_syscall, number, SYS_capget);
    will_return_count(__wrap_syscall, INH_CAP_MASK_DONT_CARE, 2);
    will_return(__wrap_syscall, 0);
    will_return(__wrap_syscall, 1 << (CAP_MAC_OVERRIDE - 32));
    assert_int_equal(crinitCapIsCapsetEffective(CAP_MAC_OVERRIDE, PID_DONT_CARE), true);
}

void test_crinitCapIsCapsetEffective_last_supported(void **state) {
    CRINIT_PARAM_UNUSED(state);

    expect_not_value(__wrap_syscall, hdr, NULL);
    expect_value(__wrap_syscall, hdr->version, _LINUX_CAPABILITY_VERSION_3);
    expect_value(__wrap_syscall, hdr->pid, PID_DONT_CARE);
    expect_not_value(__wrap_syscall, out, NULL);
    expect_value(__wrap_syscall, number, SYS_capget);
    will_return_count(__wrap_syscall, INH_CAP_MASK_DONT_CARE, 2);
    will_return(__wrap_syscall, 0);
    will_return(__wrap_syscall, 1 << (CAP_LAST_CAP - 32));
    assert_int_equal(crinitCapIsCapsetEffective(CAP_LAST_CAP, PID_DONT_CARE), true);
}

void test_crinitCapIsCapsetEffective_not_set(void **state) {
    CRINIT_PARAM_UNUSED(state);

    expect_not_value(__wrap_syscall, hdr, NULL);
    expect_value(__wrap_syscall, hdr->version, _LINUX_CAPABILITY_VERSION_3);
    expect_value(__wrap_syscall, hdr->pid, PID_DONT_CARE);
    expect_not_value(__wrap_syscall, out, NULL);
    expect_value(__wrap_syscall, number, SYS_capget);
    will_return_count(__wrap_syscall, INH_CAP_MASK_DONT_CARE, 2);
    will_return(__wrap_syscall, ~(1 << CAP_CHOWN));
    will_return(__wrap_syscall, 0x0);
    assert_int_equal(crinitCapIsCapsetEffective(CAP_CHOWN, PID_DONT_CARE), false);
}
