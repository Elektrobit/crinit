// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitCapGetInheritable(), successful execution.
 */

#include <sys/syscall.h>

#include "capabilities.h"
#include "common.h"
#include "unit_test.h"
#include "utest-crinit-cap-get-inheritable.h"

#define EFF_CAP_MASK_DONT_CARE 0
#define PID_DONT_CARE 0

void test_crinitCapGetInheritable(void **state) {
    CRINIT_PARAM_UNUSED(state);
    uint64_t result = 0;

    expect_not_value(__wrap_syscall, hdr, NULL);
    expect_value(__wrap_syscall, hdr->version, _LINUX_CAPABILITY_VERSION_3);
    expect_value(__wrap_syscall, hdr->pid, PID_DONT_CARE);
    expect_not_value(__wrap_syscall, out, NULL);
    expect_value(__wrap_syscall, number, SYS_capget);
    will_return(__wrap_syscall, 0x80000002);
    will_return(__wrap_syscall, 0x80000001);
    will_return_count(__wrap_syscall, EFF_CAP_MASK_DONT_CARE, 2);
    assert_int_equal(crinitCapGetInheritable(PID_DONT_CARE, &result), 0);
}

void test_crinitCapGetInheritable_resultParmInitialized(void **state) {
    CRINIT_PARAM_UNUSED(state);
    uint64_t result = 0;

    expect_not_value(__wrap_syscall, hdr, NULL);
    expect_value(__wrap_syscall, hdr->version, _LINUX_CAPABILITY_VERSION_3);
    expect_value(__wrap_syscall, hdr->pid, PID_DONT_CARE);
    expect_not_value(__wrap_syscall, out, NULL);
    expect_value(__wrap_syscall, number, SYS_capget);
    will_return(__wrap_syscall, (1 << CAP_SYS_BOOT) | (1 << CAP_SYS_CHROOT));
    will_return(__wrap_syscall, (1 << (CAP_BPF - 32)) | (1 << (CAP_AUDIT_READ - 32)));
    will_return_count(__wrap_syscall, EFF_CAP_MASK_DONT_CARE, 2);
    result = 1;  // expect that this result initialization will be overwritten (i.e. Bit 0 won't be set)
    assert_int_equal(crinitCapGetInheritable(PID_DONT_CARE, &result), 0);
    assert_int_equal(result, (1uLL << CAP_BPF) | (1uLL << CAP_AUDIT_READ) | (1uLL << CAP_SYS_BOOT) |
                                 (1uLL << CAP_SYS_CHROOT));  // lowest bit 0 is not set
}
