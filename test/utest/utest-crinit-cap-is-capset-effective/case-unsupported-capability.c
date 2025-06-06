// SPDX-License-Identifier: MIT
/**
 * @file case-unsupported-capability.c
 * @brief Unit test for crinitCapIsCapsetEffective(), successful execution.
 */

#include <sys/syscall.h>

#include "capabilities.h"
#include "common.h"
#include "unit_test.h"
#include "utest-crinit-cap-is-capset-effective.h"

#define INH_CAP_MASK_DONT_CARE 0
#define PID_DONT_CARE 0

void test_crinitCapIsCapsetEffective_unsupported_capability(void **state) {
    CRINIT_PARAM_UNUSED(state);

    expect_not_value(__wrap_syscall, hdr, NULL);
    expect_value(__wrap_syscall, hdr->version, _LINUX_CAPABILITY_VERSION_3);
    expect_value(__wrap_syscall, hdr->pid, PID_DONT_CARE);
    expect_not_value(__wrap_syscall, out, NULL);
    expect_value(__wrap_syscall, number, SYS_capget);
    will_return_count(__wrap_syscall, INH_CAP_MASK_DONT_CARE, 2);
    will_return(__wrap_syscall, 0x0);
    will_return(__wrap_syscall, ~(1 << (CAP_MAC_ADMIN - 32)));
    // bit next to LSB of second capability array element set
    assert_int_equal(crinitCapIsCapsetEffective(CAP_MAC_ADMIN, PID_DONT_CARE), false);
}

void test_crinitCapIsCapsetEffective_first_after_last_capability(void **state) {
    CRINIT_PARAM_UNUSED(state);
    // first unsupported capability: return false although the capability bit was mocked (but it's eventually
    // unsupported)
    assert_int_equal(crinitCapIsCapsetEffective(CAP_LAST_CAP + 1, PID_DONT_CARE), false);
}

void test_crinitCapIsCapsetEffective_last_possible_capability(void **state) {
    CRINIT_PARAM_UNUSED(state);
    // first unsupported capability: return false although the capability bit was mocked (but it's eventually
    // unsupported)
    assert_int_equal(crinitCapIsCapsetEffective(63, PID_DONT_CARE), false);
}
